# TraCAR
 --- A Bluetooth car automatically controlled by a PC with a webcam to trace black lines.

### Video 1 

[![TraCAR-demo-1-en](http://img.youtube.com/vi/ktlbX7IkARQ/0.jpg)](http://www.youtube.com/watch?v=ktlbX7IkARQ)

### Video 2 

[![TraCAR-demo-dorm-en](http://img.youtube.com/vi/PC991xsAWYM/0.jpg)](http://www.youtube.com/watch?v=PC991xsAWYM)



## Introduction
**TraCAR** is a course project for *(EI228) Science and Technology Innocation (Part 2-B)* (Spring 2016).

The project is composed of *a self-made car* with an Arduino microcontroller and *a PC program* utilizing OpenCV in C++.
After initialization, the PC program will automatically capture and process the image of the "runway" (black lines) from the webcam and control the car to go along the black line in real-time via Bluetooth. 


## File Structure
PC-end/				*files for the PC program*
PC-end/build/		*an executable file for the PC program*
PC-end/src/			*the source code for the PC program*
PC-end/vcproj/		*Visual C++ project directory*
car-end/src/		*Arduino source code*
website/			*files for the website*


## Webpage

[http://eelab.sjtu.edu.cn/kc/2016-06/B25/](http://eelab.sjtu.edu.cn/kc/2016-06/B25/ "TraCAR Report Webpage (in Chinese)")

The link above points to the report webpage (in Chinese) with implementation details. 

(Possibly a device may need some time loading the fonts for the first time.)


## Contribution
Focus on OpenCV code prototype development, the hardware of the Bluetooth car, joint debugging and report webpage.

Lead the development of the entire project.


## Details(selected)

* The PC-end program is implemented in C++ with OpenCV 2.4.

* PC and the car are connected via Bluetooth. At the PC-end, related Windows APIs are called for the Bluetooth communication. An HC-06 Bluetooth module is used for the car, which is connected to the microcontroller (Arduino) via serial ports.

* Figure 1 & 2 presents an overview of our self-made car.

![Missing image](https://github.com/WMBao/TraCAR/blob/master/website/img/7.jpg)
![Missing image](https://github.com/WMBao/TraCAR/blob/master/website/img/6.jpg)


* Figure 3 illustrates the connections among Arduino system modules.

![Missing image](https://github.com/WMBao/TraCAR/blob/master/website/img/5.jpg)

* Once an image is captured, sequentially the following steps are conducted.
	- Perspective transformation
	- Image binarization
	- Line thinning
	- Corner detection

* The car is marked with two colored points, a red point indicating the head and a blue point indicating its tail.
