# skew-biology
Software for my DYI spectrometers, sensors, pcr, and incubators.

## Spectrometer Software
Software for reading a spectrum from a DYI spectrometer using OpenCV, 
calibrating it using non-linear regression to the function with the gnu scientific library.

#### Example DYI Lego Spectrometer
* [1k diffraction grating](https://www.amazon.com/gp/product/B0074R74D8/ref=oh_aui_detailpage_o09_s00?ie=UTF8&psc=1)
* [Sony IMX179 8MP CCD](https://www.amazon.com/gp/product/B01HD1V1Z6/ref=oh_aui_search_detailpage?ie=UTF8&psc=1)
* Velcro, aluminum foil, and a lot of black legos
* CCD at m=0
![alt text](https://github.com/jkew/skew-biology/raw/master/spec-lego.jpg "LEGO Spectrometer" | width=100)

### Usage: 
```spec [roi x y w h] [cal C0 C1 C2]```

The ROI and constants for calibration can be specified on the command line. The calculated intensity and wavelength can be saved to csv alongside the calibration file for analysis in other programs.

### Calibration Details
This software currently uses a second-order polynomial for calibration off a minimum of three points. There may be value in moving to a cubic function.

   nm = C0 + C1*p + C2*p^2 

#### Calibration References
* [Calibrating the Wavelength of Your Spectrometer](http://www.coseti.org/pc2000_2.htm)
* [OceanOptics Cubic Calibration](https://publiclab.org/notes/wiebew/12-30-2012/spectrometer-calibration)
* [PublicLab Linear Calibration](https://publiclab.org/notes/wiebew/12-30-2012/spectrometer-calibration)

### Selecting a Region of Interest
On startup the region of interest can be selected with a mouse. The pixels in this region of interest are summed to calculate intensity.

### Commands:
| Command | Note 
|---------| -----
| top     | lists the top wavelengths, intensities, and pixel indicies
| cal     | enters calibration mode
| save    | saves to csv along with the calibration

### Building:
```
cmake .
make
```

### Example usage:
Calibrating a CFL using the mercury peaks terbium peaks.
![alt text](https://github.com/jkew/skew-biology/raw/master/spec-example.png "Example calibration and usage" | width=250)




# Incubator
PID controled incubator w/ IR

# RGB
simple rgb sensor control for recording the rgb values of a sample 
being incubated by a connected computer

