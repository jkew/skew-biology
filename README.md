# skew-biology
tools and software for my spectrometers, sensors, pcr, and incubators

## DYI Spectrometer Calibration 
Basic software for reading a spectrum from a DYI spectrometer using OpenCV, 
calibrating it using non-linear regression to the function with the gnu scientific library:

   nm = C_{0} + C_{1}p + C_{2}p^2 

On startup the region of interest can be selected with a mouse. The pixels in this region of interest are summed to calculate intensity.

The ROI and constants for calibration can be specified on the command line. The calculated intensity and wavelength can be saved to csv alongside the calibration file for analysis in other programs.

### Usage: 
```spec [roi x y w h] [cal C_{0} C_{1} C_{2}]```

### Building:
```
cmake .
make
```

### Example usage:
![alt text](https://github.com/jkew/skew-biology/raw/master/spec-example.png "Example calibration and usage")




# Incubator
PID controled incubator w/ IR

# RGB
simple rgb sensor control for recording the rgb values of a sample 
being incubated by a connected computer

