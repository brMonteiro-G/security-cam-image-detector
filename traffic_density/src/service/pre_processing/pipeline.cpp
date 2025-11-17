
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>

using namespace cv;
using namespace std;


int main( int argc, char** argv )
{
    string imageName = "my_image.jpg";  
    Mat src = imread(imageName, IMREAD_COLOR);
    if( src.empty() )
      {
          cout << "Could not open or find the image!\n" << endl;
          cout << "Usage: " << argv[0] << " <Input image>" << endl;
          return -1;
      }
    Mat sharpened_image;
    Mat kernel = (Mat_<double>(3, 3) << -1, -1, -1,
                                         -1,  9, -1,
                                         -1, -1, -1);
    filter2D(image, sharpened_image, -1, kernel); 
    
}
