#include "opencv2/opencv.hpp"
#include "opencv2/plot.hpp"
#include <iostream>
#include <unistd.h>
#include <vector>
#include <thread>
#include <mutex>

using namespace cv;

std::mutex mtx;

VideoCapture cap(0); // open the default camera
Rect2d roi(0, 0, 0, 0);
std::vector<double> signals;
std::vector<std::pair<int, int> >calibration;
bool running = true;

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
  std::cout << "Rank:\tIndex, Signal" << std::endl;
  for (int i = 0; i < k; ++i) {
    int ki = q.top().second;
    double val = q.top().first;
    std::cout << i << ":\t (" << ki << ", " << val <<  ")" << std::endl;
    q.pop();
  }
}

void calibrate() {
  while(running) {
    std::string cmd;  
    std::cout << "CALIBRATE[print|clear|add|exit]" << std::endl << "> ";
    getline(std::cin, cmd);
    if (cmd.compare(std::string("print")) == 0) {
        for (int i = 0; i < calibration.size(); ++i) {
	  std::cout << "Index: " << calibration[i].first << " nm: " << calibration[i].second << std::endl;  
	}
    }
    if (cmd.compare(std::string("exit")) == 0) {
      break;
    }
    if (cmd.compare(std::string("clear")) == 0) {
      calibration.clear();
      std::cout << "cleared" << std::endl;
    }

    if (cmd.compare(std::string("add")) == 0) {
      std::cout << "\tindex> ";
      getline(std::cin, cmd);
      int idx, nm = 0;
      if (cmd.size() > 0) {
	idx = stoi(cmd);
      } else {
	continue;
      }
      std::cout << "\tnm> ";
      getline(std::cin, cmd);
      if (cmd.size() > 0) {
	nm = stoi(cmd);
      } else {
	continue;
      }
      calibration.push_back(std::pair<double, int>(idx, nm));
    }
  }
}

void command() {
  while(running) {
    std::string cmd;
    std::cout << "[?|cal|save|roi|top|quit]" << std::endl << "> ";
    getline(std::cin, cmd);
    if (cmd.compare(std::string("?")) == 0) {
      std::cout << "cal  : UNIMP calibrate" << std::endl;
      std::cout << "save : UNIMP save data" << std::endl;
      std::cout << "roi  : UNIMP set region of interest" << std::endl;
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

  if (roi.width == 0) {
    set_roi();
  }

  std::cout << "ROI: " << roi.x << " " << roi.y << " " << roi.width << " " << roi.height << std::endl;
  std::thread t1(command);
  render();
  // the camera will be deinitialized automatically in VideoCapture destructor
  return 0;
}
