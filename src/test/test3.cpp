#include "../image.h"
#include "../utils.h"
#include "../matrix.h"

#include <string>

using namespace std;



void test_structure()
  {
  Image im = load_image("data/dogbw.png");
  Image s = structure_matrix(im, 2);
  s.feature_normalize_total();
  save_png(s, "output/structure");
  Image gt = load_image("data/structure.png");
  
  TEST(same_image(s, gt));
  }

void test_cornerness()
  {
  Image im = load_image("data/dogbw.png");
  Image s = structure_matrix(im, 2);
  Image c = cornerness_response(s,0);
  c.feature_normalize_total();
  save_png(c, "output/response");
  Image gt = load_image("data/response.png");
  TEST(same_image(c, gt));
  }




struct LKIterPyramid
  {
  // INPUT
  vector<Image> pyramid1;
  vector<Image> pyramid0;
  Image t0;
  Image t1;
  
  // OUTPUT
  Image v;  // resulting velocity
  Image colorflow; // colorized velocity
  Image error; // difference between warp(t0,v) and t1
  Image ev3; // three channel eigenvalue image for displaying
  Image warped;// warped t0 with flow v
  Image all; // ciombined 4 images for Opencv display
  
  // OPTIONS
  float subsample_input=2;    // how much to reduce input image size
  float smooth_structure=1;   // how much to smooth structure matrix
  float smooth_vel=1;         // how much to smooth resulting velocity
  int lk_iterations=2;        // LK iterations to run (0 -  no flow, 1 - standard version)
  int pyramid_levels=6;       // pyramid levels (1 - standard algo)
  float pyramid_factor=2;     // ratio of sizes between successive pyramid levels
  float clamp_vel=10;         // how much to clamp velocity
  float vel_color_scale=4;    // saturation of vel image
  
  bool compute_all=false; // compute total combined image for opencv
  bool compute_colored_ev=true; // compute colored eigenvalue image
  };


void compute_iterative_pyramid_LK(LKIterPyramid& lk)
  {
  TIME(1); // time algo
  Image S;
  Image ev;
  Image v2;
  
  int h=lk.pyramid0[0].h;
  int w=lk.pyramid0[0].w;
  
  for(int q2=lk.pyramid_levels-1;q2>=0;q2--)
    {
    
    int pw=lk.pyramid1[q2].w;
    int ph=lk.pyramid1[q2].h;
    
    
    if(q2==lk.pyramid_levels-1)
      {
      lk.v=Image(pw,ph,2);
      lk.warped=lk.pyramid0[q2];
      }
    else
      {
      lk.v=velocity_resize(lk.v,pw,ph);
      lk.warped=warp_flow(lk.pyramid0[q2],lk.v);
      }
    
    for(int q1=0;q1<lk.lk_iterations;q1++)
      {
      S = time_structure_matrix(lk.pyramid1[q2], lk.warped, lk.smooth_structure);
      ev = eigenvalue_matrix(S);
      v2 = velocity_image(S, ev);
      
      v2=fast_smooth_image(v2,lk.smooth_vel);
      lk.v=lk.v+v2;
      
      constrain_image(lk.v,lk.clamp_vel);
      lk.warped=warp_flow(lk.pyramid0[q2],lk.v);
      }
    
    }
  
  
  
  lk.colorflow=vel2rgb(lk.v,lk.vel_color_scale);
  lk.error=(lk.warped-lk.pyramid1[0]).abs();
  
  if(lk.compute_all)
    {
    lk.all=Image(w*2,h*2,3);
    for(int c=0;c<3;c++)for(int q2=0;q2<h;q2++)for(int q1=0;q1<w;q1++)lk.all(q1+0,q2+0,c)=lk.t1(q1,q2,c);
    for(int c=0;c<3;c++)for(int q2=0;q2<h;q2++)for(int q1=0;q1<w;q1++)lk.all(q1+w,q2+0,c)=lk.colorflow(q1,q2,c);
    for(int c=0;c<3;c++)for(int q2=0;q2<h;q2++)for(int q1=0;q1<w;q1++)lk.all(q1+0,q2+h,c)=lk.warped(q1,q2);
    for(int c=0;c<3;c++)for(int q2=0;q2<h;q2++)for(int q1=0;q1<w;q1++)lk.all(q1+w,q2+h,c)=lk.error(q1,q2);
    }
  
  if(lk.compute_colored_ev)
    {
    lk.ev3=Image(ev.w,ev.h,3);
    memcpy(lk.ev3.data,ev.data,ev.size()*sizeof(float));
    }
  
  }






// Run optical flow demo on webcam
void optical_flow_webcam(void)
  {
  LKIterPyramid lk;
  lk.compute_all=true;
  lk.compute_colored_ev=false;
  
  
#ifdef OPENCV
  cv::VideoCapture cap(0);
  assert(cap.isOpened() && "cannot open camera");
  Image imo[2]; // original full size image
  Image im[2]; // resized image
  vector < Image > pyramid[2]; // pyramids
  
  int cur=1; // current latest image (1-cur is the previous image)
  
  imo[cur]=get_image_from_stream(cap);
  
  int w=imo[cur].w/lk.subsample_input;
  int h=imo[cur].h/lk.subsample_input;
  
  im[cur]=bilinear_resize(fast_smooth_image(imo[cur],lk.subsample_input/2),w,h);
  pyramid[cur]=make_image_pyramid(im[cur].rgb_to_grayscale(),lk.pyramid_factor,lk.pyramid_levels);
  
  
  
  
  //Image all(w*2,h*2,3);
  //Image warped,v;
  
  while(1)
    {
    
    
    
    cur=1-cur;
    
    imo[cur]=get_image_from_stream(cap);
    
    
    
    im[cur]=bilinear_resize(fast_smooth_image(imo[cur],lk.subsample_input/2),w,h);
    pyramid[cur]=make_image_pyramid(im[cur].rgb_to_grayscale(),lk.pyramid_factor,lk.pyramid_levels);
    
    {
    lk.t1=im[cur];               // init
    lk.t0=im[1-cur];             // init
    lk.pyramid1=pyramid[cur];    // init
    lk.pyramid0=pyramid[1-cur];  // init
    }
    
    compute_iterative_pyramid_LK(lk);
    
    cv::imshow("Optical-Flow",Image2Mat(lk.all));
    if( cv::waitKey(1) == 27 ) break; // stop capturing by pressing ESC 
    
    
    
    //Image copy=im;
    //Image v = optical_flow_images(im_c.rgb_to_grayscale(), prev_c.rgb_to_grayscale(), 2,2);
    ////draw_flow(copy, v, smooth*div);
    ////save_image(copy,"copy");
    //int key = show_image(copy, "flow", 5);
    //prev=move(im);
    //prev_c=move(im_c);
    //
    //if(key != -1){key = key % 256;printf("%d\n", key);if (key == 27) break;}
    //
    //im = get_image_from_stream(cap);
    //im_c = nn_resize(im, im.w/div, im.h/div);
    }
#else
    fprintf(stderr, "Must compile with OpenCV\n");
#endif
  }


void run_tests()
  {
  //test_structure();
  //test_cornerness();
  
  //printf("%d tests, %d passed, %d failed\n", tests_total, tests_total-tests_fail, tests_fail);
  }

int main(int argc, char **argv)
  {
  //run_tests();
  
  //optical_flow_webcam();
  
  Image a = load_image("data/dog_a.jpg");
  Image b = load_image("data/dog_b.jpg");
  Image ag = a.rgb_to_grayscale();
  Image bg = b.rgb_to_grayscale();
  Image flow = optical_flow_images(bg, ag,5,5);
  Image colorflow=vel2rgb(flow,10);
  save_image(colorflow, "output/dog_vel");
  
  LKIterPyramid lk;
  lk.pyramid_levels=8;
  lk.vel_color_scale=20;
  lk.clamp_vel=50;
  
  lk.t1=b;
  lk.t0=a;
  lk.pyramid0=make_image_pyramid(ag,lk.pyramid_factor,lk.pyramid_levels);
  lk.pyramid1=make_image_pyramid(bg,lk.pyramid_factor,lk.pyramid_levels);
  
  compute_iterative_pyramid_LK(lk);
  
  save_image(lk.colorflow, "output/dog_vel_improved");
  
  
  return 0;
  }
