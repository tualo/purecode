std::vector<cv::Mat> Image::findDMTXRectangles(cv::Mat &gray, bool bc){

    std::vector<cv::Mat> results={};
    // compute mask (you could use a simple threshold if the image is always as good as the one you provided)
    cv::Mat mask;
    cv::Mat tgray;



/*
    dilate(mask, mask, 0, cv::Point(-1, -1), 2, 1, 1);
*/

    cv::Size ksize(15,15);
    cv::GaussianBlur(gray, tgray, ksize, 0);
    cv::threshold(tgray, mask, 0, 255, CV_THRESH_BINARY_INV | CV_THRESH_OTSU);
    cv::erode(mask, mask, cv::Mat(),cv::Point(-1,-1), 5);
    cv::dilate(mask, mask, cv::Mat(),cv::Point(-1,-1), 15);



    // find contours (if always so easy to segment as your image, you could just add the black/rect pixels to a vector)
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    
//  cv::findContours(mask,contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    cv::findContours(mask,contours, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

    /// Draw contours and find biggest contour (if there are other contours in the image, we assume the biggest one is the desired rect)
    // drawing here is only for demonstration!
    int biggestContourIdx = -1;
    float biggestContourArea = 0;
    cv::Mat drawing = cv::Mat::zeros( mask.size(), CV_8UC3 );
    for( int i = 0; i< contours.size(); i++ )
    {
        cv::Scalar color = cv::Scalar(0, 100, 0);
        drawContours( drawing, contours, i, color, 1, 8, hierarchy, 0, cv::Point() );

        float ctArea= cv::contourArea(contours[i]);
        
        if ((ctArea > 1.2)&&ctArea*2<(gray.cols*gray.rows)){
            if(ctArea > biggestContourArea)
            {
                biggestContourArea = ctArea;
                biggestContourIdx = i;
            }
            
            // compute the rotated bounding rect of the biggest contour! (this is the part that does what you want/need)
            cv::RotatedRect boundingBox = cv::minAreaRect(contours[i]);



            
            // one thing to remark: this will compute the OUTER boundary box, so maybe you have to erode/dilate if you want something between the ragged lines
            // draw the rotated rect
            cv::Point2f corners[4];
            boundingBox.points(corners);
            cv::line(drawing, corners[0], corners[1], cv::Scalar(255,255,255));
            cv::line(drawing, corners[1], corners[2], cv::Scalar(255,255,255));
            cv::line(drawing, corners[2], corners[3], cv::Scalar(255,255,255));
            cv::line(drawing, corners[3], corners[0], cv::Scalar(255,255,255));
            
            try{
                cv::Rect brect = boundingBox.boundingRect();

               brect.x-=10;
               brect.y-=10;
               brect.width+=20;
               brect.height+=20;
                if (brect.x<0) brect.x = 0;
                if (brect.y<0) brect.y = 0;
                if (brect.width>=gray.cols)  brect.width=gray.cols;
                if (brect.height>=gray.rows) brect.height=gray.rows;

                
                /*
                if (0 < brect.x || 0 <= brect.width || brect.x + brect.width > gray.cols || 0 < brect.y ||  brect.y + brect.height > gray.rows){

                }else{
                */    

                cv::Mat imgx = gray(brect);
                
                
                if ( (imgx.cols>45) && (imgx.rows>45) ){
                    /*
                    std::cout << " brect.x " << brect.x << std::endl;
                    std::cout << " brect.y " << brect.y << std::endl;
                    std::cout << " brect.width " << brect.width << std::endl;
                    std::cout << " brect.height " << brect.height << std::endl;
                    std::cout << " w/h " << imgx.cols << "*" << imgx.rows << std::endl;
                    */
                    results.push_back(imgx);
                }
                //}
            }catch(cv::Exception cv_error){
                // std::cerr << "findDMTXRectangles()" << cv_error.msg << std::endl;
            }
        }
    }

    // if no contour found
    if(biggestContourIdx < 0)
    {
        std::cout << "no contour found" << std::endl;
        return results;
    }

    
    // compute the rotated bounding rect of the biggest contour! (this is the part that does what you want/need)
    cv::RotatedRect boundingBox = cv::minAreaRect(contours[biggestContourIdx]);
    // one thing to remark: this will compute the OUTER boundary box, so maybe you have to erode/dilate if you want something between the ragged lines


    // draw the rotated rect
    cv::Point2f corners[4];
    boundingBox.points(corners);
    cv::line(drawing, corners[0], corners[1], cv::Scalar(255,255,255));
    cv::line(drawing, corners[1], corners[2], cv::Scalar(255,255,255));
    cv::line(drawing, corners[2], corners[3], cv::Scalar(255,255,255));
    cv::line(drawing, corners[3], corners[0], cv::Scalar(255,255,255));

   
    showImage(drawing,10000);
    
    return results;
}





std::vector<cv::Mat> Image::regions(cv::Mat &inputImage){

  std::vector<cv::Mat> result;
  /**/  
  if (debugRegions)
    showImage();

  // compute the Scharr gradient magnitude representation of the images
	// in both the x and y direction
  cv::Mat gradX;
  cv::Mat gradY;
  cv::Mat gradient;
  cv::Mat blurred;
  cv::Mat thresh;
  cv::Mat closed;
  cv::Mat kernel;
  cv::Mat bwImage;


  int dx = 1;
  int dy = 0;
  cv::Size ksize = cv::Size(5,5);

  cv::Sobel(inputImage, gradX, CV_32F, dx , dy, -1);
    //showImage(gradX);

  cv::Sobel(inputImage, gradY, CV_32F, 0, 1, -1);
  //showImage(gradY);

  cv::subtract(gradX,gradY,gradient);
  //showImage(gradient);
  
  cv::blur(gradient,blurred,ksize);
  //showImage(blurred);


  cv::threshold(blurred,thresh,125,255,cv::THRESH_BINARY);
  //showImage(thresh);


  kernel = cv::getStructuringElement(cv::MORPH_RECT,cv::Size(31,31));
  //showImage(kernel);

  cv::morphologyEx(thresh,closed,cv::MORPH_CLOSE,kernel);
  if (debugRegions)
    showImage(closed);

  // perform a series of erosions and dilations
	//cv::erode(closed, closed, cv::Mat(),cv::Point(-1,-1), 24);
  std::vector<int> erodeList = {1,5,15,35,45};  
  std::vector<std::vector<cv::Point> > contours;
  std::vector<cv::Vec4i> hierarchy;
  double erode_contours = 0;
  double dilate_contours = 0;
  double qoute = 0;

  for( size_t i = 0; i< erodeList.size()&&qoute<0.3; i++ )
  {
    cv::erode(closed, closed, cv::Mat(),cv::Point(-1,-1), erodeList.at(i));
    //if (debugRegions)
    //  showImage(closed,100);


    closed.convertTo(bwImage, CV_8UC1);
    cv::findContours(bwImage, contours, hierarchy, cv::RETR_TREE,cv::CHAIN_APPROX_SIMPLE);
    erode_contours = (double)contours.size();
    std::cout << "contours: " << contours.size() << std::endl;

    //cv::dilate(closed, closed, cv::Mat(),cv::Point(-1,-1), 50);
    cv::dilate(closed, closed, cv::Mat(),cv::Point(-1,-1), 20);
    //if (debugRegions)
    // showImage(closed,100);



    closed.convertTo(bwImage, CV_8UC1);

    cv::findContours(bwImage, contours, hierarchy, cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);
    
    std::vector<cv::RotatedRect> minRect( contours.size() );

    dilate_contours = (double)contours.size();
    std::cout << "contours: " << contours.size() << std::endl;
    std::cout << "erode: " << erodeList.at(i)<< std::endl;
    std::cout << "qoute: " << dilate_contours/erode_contours << std::endl;
    if ((dilate_contours==1) && (erode_contours==1)){
    qoute = 0;
    }else{
    qoute = dilate_contours/erode_contours;
    }
  }

  for( size_t i = 0; i< contours.size(); i++ )
  {
    cv::Scalar blue_color = cv::Scalar(255.0, 0.0, 0.0); 
    cv::Scalar green_color = cv::Scalar(0.0, 255.0, 0.0);
    cv::RotatedRect rotatedRect = cv::minAreaRect(contours.at(i));
    cv::Rect boundingRect = cv::boundingRect(contours.at(i));

    cv::Point2f vertices2f[4];
    rotatedRect.points(vertices2f);
    
    cv::Point vertices[4];    
    for(int i = 0; i < 4; ++i){
        vertices[i] = vertices2f[i];
    }


    cv::Point2f rect_points[4]; 
    rotatedRect.points( rect_points );
    for( int j = 0; j < 4; j++ )
      cv::line( debugImage, rect_points[j], rect_points[(j+1)%4], blue_color, 3, 8 );

    /*cv::fillConvexPoly(debugImage,
                    vertices,
                    4,
                    blue_color);
    */
    
    cv::Point inflationPoint(-20, -20);
    cv::Size inflationSize(40, 40);

    boundingRect += inflationPoint;
    boundingRect += inflationSize;
    if (boundingRect.x<0)boundingRect.x=0;
    if (boundingRect.y<0)boundingRect.y=0;
    if (boundingRect.x+boundingRect.width>originalImage.cols){
      boundingRect.width -= boundingRect.x+boundingRect.width- originalImage.cols;
    }
    if (boundingRect.y+boundingRect.height>originalImage.rows){
      boundingRect.height -= boundingRect.y+boundingRect.height- originalImage.rows;
    }

    cv::rectangle(debugImage,boundingRect,green_color,3);
    std::cout << "br: " << boundingRect.x << ", " << boundingRect.y << "wh: " << boundingRect.width << ", " << boundingRect.height   << std::endl;
    try
    {
        result.push_back(inputImage(boundingRect));
    }
    catch( cv::Exception& e )
    {
    }
  }

  //showDebugWindowWaitTime=5000;
  if (debugRegions)
    showImage(debugImage,5000);
  return result;
}
