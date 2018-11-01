# CSE 455 Homework 3 #

Welcome friends,

It's time for optical flow!

To start out this homework, copy over your `process_image.cpp`, `filter_image.cpp`, `resize_image.cpp`, `harris_image.cpp`, and`panorama_image.cpp` files from hw2 to the `src` directory in this homework. We will be continuing to build out your image library.

## 1. Lucas-Kanade optical flow ##

We'll be implementing [Lucas-Kanade](https://en.wikipedia.org/wiki/Lucas%E2%80%93Kanade_method) optical flow. We'll use a structure matrix but this time with temporal information as well. The equation we'll use is:

![](figs/flow-eq.png)

Note the first matrix consists of 3 different elements and corresponds to `S'S` from the lecture slides and the second term is a 2-vector corresponding to `S'T`. We put the resulting 5 numbers in the so called Time-structure-matrix.

## 1.1 Time-structure matrix ##

We'll need spatial and temporal gradient information for the flow equations. Calculate a time-structure matrix. Spatial gradients can be calculated as normal. The time gradient can be calculated as the difference between the previous image and the next image in a sequence. Fill in `time_structure_matrix`. At the end filter the resulting matrix with a `sigma=s`. Use the provided function `fast_smooth_image` for fast gaussian smoothing.

## 1.2 Calculate the eigenvalues of `S'S` ##

To know whether we have enough information to compute the optical flow we need to know whether the matrix `S'S` is invertible.
Compute the two eigenvalues per pixel and return an image with two channels

## 1.3 Calculating velocity from the structure matrix ##

Fill in `velocity_image` to use the equation to calculate the velocity of each pixel in the x and y direction. For each pixel, produce a matrix `S'S` and a vector `S'T` using the classes `class Matrix2x2` and `class Vector2`. Invert the matrix, and use it to calculate the velocity.

Try calculating the optical flow between two dog images

    Image a = load_image("data/dog_a.jpg");
    Image b = load_image("data/dog_b.jpg");
    Image flow = optical_flow_images(b, a)
    draw_flow(a, flow, 8)
    save_image(a, "lines")

It may look something like:

![](figs/lines.jpg)

## 2. Implement Iterative refinement and Pyramidal refinement ##

For your convenience computing the sequence of pyramidal images has been implemented. You need to implement two methods. 

First is `velocity_resize`. You computed the velocity image at a lower resolution. Now you move down the pyramid and need to initialize the flow with the flow from the image below. The function takes the old flow and the new sizes which are bigger and you need to return the updated flow. Think about what rescaling/resizing the flow means. Is it just image resizing?

After you have updated flow, you can run iterative LK. That means running one iteration of LK, obtainig a flow `v` from image `t0` to `t1`, and warping `t0` accoring to the flow `v`. That means sending each pixel from `t0(x,y)` to the location `(x+vx,y+dy)=(x+v(x,y,0),y+(v,x,y,1))`. Think what is necessary to achieve that. What do you do if a pixel does not move exactly to a new integer coordinates pixel? What do you if no pixel goes to a given new pixel `(x,y)`? You can answer and implement these questions using techniques we studied in class.

## 3. Optical flow demo using OpenCV or Pangolin ##

Using OpenCV and Pangolin we can get images from the webcam and display the results in real-time. 

## 34. Turn it in ##

Turn in your `flow_image.cpp` on canvas under Assignment 3.
