#include "opencv2/opencv.hpp"
#include "opencv2/plot.hpp"
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <vector>
#include <thread>
#include <mutex>
#include <gsl/gsl_multifit.h>

using namespace cv;

std::mutex mtx;

VideoCapture cap(0); // open the default camera
Rect2d roi(0, 0, 0, 0);
std::vector<double> signals;
std::vector<std::pair<int, int> >calibration;
bool running = true;

// Polynomial fit for calibration
// y = c_0 + c_1 x + c_2 x^2
// http://www.coseti.org/pc2000_2.htm
// https://www.gnu.org/software/gsl/manual/html_node/Fitting-Examples-for-multi_002dparameter-linear-regression.html
double c1 = 0, c2 = 1, c3 = 0;

int eval_nm(int px) {
  return (int) c1 + c2 * px + c3 * px * px;
}

void gsl_fit() {
  int numPoints = calibration.size();
  double xi, yi, ei, chisq;
  gsl_matrix *X, *cov;
  gsl_vector *y, *w, *c;

  X = gsl_matrix_alloc (numPoints, 3);
  y = gsl_vector_alloc (numPoints);
  w = gsl_vector_alloc (numPoints);

  c = gsl_vector_alloc (3);
  cov = gsl_matrix_alloc (3, 3);

  for (int i = 0; i < numPoints; i++) {
      xi = calibration[i].first;
      yi = calibration[i].second;
      ei = 1; // error

      gsl_matrix_set (X, i, 0, 1.0);
      gsl_matrix_set (X, i, 1, xi);
      gsl_matrix_set (X, i, 2, xi*xi);
      
      gsl_vector_set (y, i, yi);
      gsl_vector_set (w, i, 1.0/(ei*ei));
  }

  {
    gsl_multifit_linear_workspace * work 
      = gsl_multifit_linear_alloc (numPoints, 3);
    gsl_multifit_wlinear (X, w, y, c, cov,
                          &chisq, work);
    gsl_multifit_linear_free (work);
  }

#define C(i) (gsl_vector_get(c,(i)))
#define COV(i,j) (gsl_matrix_get(cov,(i),(j)))

  {
    c1 = C(0);
    c2 = C(1);
    c3 = C(2);
    std::cout << "Best Fit: nm = " << C(0) << "+" << C(1) << "P+" << C(2) << "P^2" << std::endl;
    std::cout << "Covariance Matrix" << std::endl;
    std::cout << std::scientific << COV(0,0) << " " << COV(0,1) << " " << COV(0,2) << std::endl;
    std::cout << std::scientific << COV(1,0) << " " << COV(1,1) << " " << COV(1,2) << std::endl;
    std::cout << std::scientific << COV(2,0) << " " << COV(2,1) << " " << COV(2,2) << std::endl;
    std::cout << " Chi SQ: " << chisq << std::endl;
  }

  gsl_matrix_free (X);
  gsl_vector_free (y);
  gsl_vector_free (w);
  gsl_vector_free (c);
  gsl_matrix_free (cov);
}

void render() {
  namedWindow("Spectrum",1);
  Mat plotDisplay;
  Ptr<plot::Plot2d> plot;
  while(running)
    {
      mtx.lock();
      Mat frame;
      cap >> frame; // get a new frame from camera
      signals.clear();

      // Crop image from ROI;
      Mat imCrop = frame(roi);
      for (int x = 0; x < roi.width; x++) {
        long signal = 0;
        for (int y = 0; y < roi.height; y++) {
	   Vec3b intensity = imCrop.at<Vec3b>(y, x);
           uchar blue = intensity.val[0];
	   uchar green = intensity.val[1];
	   uchar red = intensity.val[2];
           signal += blue;
           signal += green;
           signal += red;
        }

	//std::cout << x << " " << signal << std::endl;
        signals.push_back(signal);
      }
      //std::cout << std::endl << std::endl;

      Mat ydata(signals);
      plot = plot::Plot2d::create(ydata);
      plot->setInvertOrientation(true);
      plot->setShowGrid(false);
      plot->setPlotTextColor(Scalar(0,0,0));
      plot->setPlotBackgroundColor(Scalar(0,0,100));
      plot->setShowText(false);
      plot->render(plotDisplay);      
      imshow("Plot", plotDisplay);
      imshow("Spectrum", imCrop);
      mtx.unlock();
      if (waitKey(30) >= 0) { break; }
    }
}

void set_roi() {
  mtx.lock();
  Mat select;
  cap >> select;
  roi = selectROI("select", select, true, false);
  destroyWindow("select");
  mtx.unlock();
}

void save() {
  std::string name;
  std::cout << "filename: ";
  getline(std::cin, name);
  std::ofstream datafile, calfile;
  datafile.open(name + ".csv");
  mtx.lock();
  datafile << "Index,nm,signal" << std::endl;
  for (int i = 0; i < signals.size(); ++i) {
    datafile << i << "," << eval_nm(i) << "," << signals[i] << std::endl;
  }
  datafile.close();
  mtx.unlock();
  calfile.open(name + ".cfg");
  calfile << "roi " << roi.x << " " << roi.y << " " << roi.width << " " << roi.height << std::endl;
  calfile << "cal " << c1 << " " << c2 << " " << c3 << std::endl;
  calfile.close();
  std::cout << "Saved data to " << name << ".csv and calibration config to " << name << ".cfg" << std::endl;
}

void top10() {
  mtx.lock();
  std::priority_queue<std::pair<double, int> > q;
  for (int i = 0; i < signals.size(); ++i) {
    // Filter out points where the neighboring index has a stronger
    // signal
    if (i > 1) {
      if (signals[i - 1] > signals[i])
	continue;
    }
    if (i < signals.size() - 1) {
      if (signals[i + 1] > signals[i])
	continue;
    }
    q.push(std::pair<double, int>(signals[i], i));
  }
  mtx.unlock();
  int k = 10;
  std::cout << "Rank:\t Index\t Wavelength\t Signal" << std::endl;
  for (int i = 0; i < k; ++i) {
    int ki = q.top().second;
    int nm = eval_nm(ki);
    double val = q.top().first;    
    std::cout << i << ":\t " << ki << "\t " << nm << "\t " << val << std::endl;
    q.pop();
  }
}

void calibrate() {
  while(running) {
    std::string cmd;
    std::cout << "CALIBRATE [?|[p]rint|clr|add|exit]" << std::endl << "> ";
    getline(std::cin, cmd, '\n');
    if (cmd.compare(std::string("p")) == 0) {
         std::cout << "Calibration Curve: nm = " << c1 << "+" << c2 << "P+" << c3 << "P^2" << std::endl;
        for (int i = 0; i < calibration.size(); ++i) {
	  std::cout << "Index: " << calibration[i].first << " nm: " << calibration[i].second << std::endl;  
	}
    }
    if (cmd.compare(std::string("exit")) == 0) {
      break;
    }
    if (cmd.compare(std::string("clr")) == 0) {
      calibration.clear();
      std::cout << "cleared" << std::endl;
    }

    if (cmd.compare(std::string("add")) == 0) {
      std::cout << "Enter calibration points (pixel and nm). Press enter to stop" << std::endl;
      while (true) {
	std::cout << "\tindex> ";
	getline(std::cin, cmd);
	int idx, nm = 0;
	if (cmd.size() > 0) {
	  idx = stoi(cmd);
	} else {
	  break;
	}
	std::cout << "\tnm> ";
	getline(std::cin, cmd);
	if (cmd.size() > 0) {
	  nm = stoi(cmd);
	} else {
	  break;
	}
	calibration.push_back(std::pair<double, int>(idx, nm));
      }
      if (calibration.size() >= 3) {
	std::cout << "Fitting points to polynomial" << std::endl;
	gsl_fit();
      } else {
	std::cout << "You need more calibration points" << std::endl;
      }
    } 

    if (cmd.compare(std::string("?")) == 0) {
      std::cout << "prnt : print calibration points" << std::endl;
      std::cout << "clr  : clear calibration points" << std::endl;
      std::cout << "add  : add calibration points" << std::endl;
      std::cout << "fit  : fit points to calibration polynomial coeficients" << std::endl;
      std::cout << "exit : exit calibration" << std::endl;
    }
  }
}

void command() {
  while(running) {
    std::string cmd;
    std::cout << "[?|cal|save|roi|top|quit]" << std::endl << "> ";
    getline(std::cin, cmd, '\n');
    if (cmd.compare(std::string("?")) == 0) {
      std::cout << "cal  : calibrate" << std::endl;
      std::cout << "save : UNIMP save data" << std::endl;
      std::cout << "top  : list positions of top ten signals" << std::endl;
      std::cout << "quit : quit" << std::endl;
      continue;
    }
    if (cmd.compare(std::string("quit")) == 0) {
      running = false;
      break;
    }
    if (cmd.compare(std::string("roi")) == 0) {
      //set_roi(); // apparently need to keep the highgui on the main thread
      std::cout << "UNIMP" << std::endl;
      continue;
    }
    if (cmd.compare(std::string("top")) == 0) {
      top10();
    }
    if (cmd.compare(std::string("cal")) == 0) {
      calibrate();
    }
    if (cmd.compare(std::string("save")) == 0) {
      save();
    }
  }
}

int main(int args, char** argv)
{
  if(!cap.isOpened())  // check if we succeeded
    return -1;

  for (int i = 1; i < args - 4; i++) {
    if (strcmp(argv[i], "roi") == 0) {
      roi.x = atoi(argv[i + 1]);
      roi.y = atoi(argv[i + 2]);
      roi.width = atoi(argv[i + 3]);
      roi.height = atoi(argv[i + 4]);
    }
  }

  for (int i = 1; i < args - 3; i++) {
    if (strcmp(argv[i], "cal") == 0) {
      c1 = atof(argv[i + 1]);
      c2 = atof(argv[i + 2]);
      c3 = atof(argv[i + 3]);
    }
  }

  if (roi.width == 0) {
    set_roi();
  }

  std::cout << "ROI: " << roi.x << " " << roi.y << " " << roi.width << " " << roi.height << std::endl;
  std::thread t1(command);
  render();
  sleep(1);
  return 0;
}
