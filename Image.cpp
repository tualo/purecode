#include "Image.h"
#include "image/rectangle.cpp"
#include "image/dmtx.cpp"



void Image::showImage(){
  showImage(debugImage,showDebugWindowWaitTime);
}


void Image::showImage(cv::Mat& src){
  showImage(src,showDebugWindowWaitTime);
}

void Image::showImage(cv::Mat& src,int ww){
  if (showDebugWindow){
    cv::Mat rotated=src.clone();
    int x=src.cols /5;
    int y=src.rows /5;
    cv::Mat res = cv::Mat(x, y, CV_8UC1);
    cv::resize(rotated, res, cv::Size(x, y), 0, 0, 3);
    cv::namedWindow("DEBUG", CV_WINDOW_AUTOSIZE );
    cv::imshow("DEBUG", res );
    cv::waitKey(ww);
  }
}


void Image::setDebug(bool val){
  showDebug=val;
}

void Image::setDebugWindow(bool val){
  showDebugWindow=val;
}

void Image::setDebugTime(bool val){
  showDebugTime=val;
}

void Image::_debugTime(std::string str){
  if (showDebugTime){
    double time_since_last = ((double)cv::getTickCount() - debug_last_time)/cv::getTickFrequency();
    std::cout << "(ImageRecognizeEx::_debugTime)\t" << str << ": " << time_since_last << "s " << std::endl;
  }
  debug_last_time = (double)cv::getTickCount();
}



Image::Image() :
  showDebug(false),
  showDebugWindow(false),
  showDebugTime(false),
  debug_last_time((double)cv::getTickCount()) {

  showDebugWindowWaitTime=500;
  subtractMean=20;

}

Image::~Image() {
  //tess->End();
}



void Image::open(const char* filename){
  fileName = filename;
  std::cout << "opening file \"" << filename << "\"." << std::endl;
  try{
    cv::setUseOptimized(true);
    cv::Mat mat = cv::imread( filename, cv::IMREAD_GRAYSCALE );
    setImage(mat);
  } catch (cv::Exception& e) {
      std::cerr << "Error opening file \"" << filename << "\". Reason: " << e.msg << std::endl;
      exit(1);
  }
}

void Image::setImage(cv::Mat mat){
  originalImage = mat.clone();
  debugImage=originalImage.clone();
  reducedImage=originalImage.clone();
  processImage=originalImage.clone();


  showImage(originalImage);
}


bool Image::is_digits(const std::string &str){
    return std::all_of(str.begin(), str.end(), ::isdigit); // C++11
}



cv::Mat Image::largestSimpleContour(cv::Mat& src){
  _debugTime("start largestSimpleContour");

  cv::Mat thr;
  cv::threshold(src, thr,subtractMean, 255,cv::THRESH_BINARY);


  int blength = thr.cols;
  int i=blength;
  int h=0;
  int height = thr.rows;


  double lastAVG = 255;
  int currentAVG = 255;
  int divisor=0;
  int avglength = 20;
  double cAVG=0;
  char avgbuffer[avglength];
  bzero(avgbuffer,avglength);
  char cavgbuffer[avglength];
  bzero(cavgbuffer,avglength);
  for(;i>199;i--){

    divisor=0;
    currentAVG=0;
    for(h=0;h<height;h+=5){
      currentAVG += thr.at<uchar>(h,i);
      divisor++;
    }
    currentAVG /=divisor;
    /*
    if (currentAVG>lastAVG*2){
      break;
    }
    */
    //std::cout << "currentAVG at i " << currentAVG << " - " << lastAVG << std::endl;
    lastAVG=0;
    cAVG=0;
    for(h=avglength-1;h>0;h--){
      avgbuffer[h]=avgbuffer[h-1];
      lastAVG+=avgbuffer[h];
    }
    for(h=avglength-1;h>0;h--){
      cavgbuffer[h]=cavgbuffer[h-1];
      cAVG+=cavgbuffer[h];
    }
    lastAVG=lastAVG/(avglength-1);
    cAVG=(cAVG+currentAVG)/(avglength);

    //std::cout << "currentAVG at i "<< i << " current " << cAVG << " - last " << lastAVG << std::endl;
    if ((i<blength-avglength*2) && (cAVG>lastAVG)) {
      //std::cout << "rising edge " << cAVG << " vs. " << lastAVG << std::endl;
      break;
    }
    avgbuffer[0]=currentAVG;
    cavgbuffer[0]=currentAVG;
  }
  //std::cout << "largestSimpleContour at i " << i << std::endl;
  if (i<200){
    std::cerr << "this should not happen the contour is to small " << i << " use the hole image "<< std::endl;
    i=blength;
  }

  cv::Rect myROI(0, 0, i, thr.rows);
  cv::Mat result = src(myROI);
  //roiImage = roiImage(myROI);

  _debugTime("stop largestSimpleContour");
  return result;
}


void Image::datamatrix(cv::Mat &image){
  DmtxTime timeout;
  DmtxImage *img;
  DmtxDecode *dec;
  DmtxRegion *reg;
  DmtxMessage *msg;
  UserOptions opt;

  int err;
  int imgScanCount, pageScanCount;

  imgScanCount = 0;
  pageScanCount = 0;
  opt = GetDefaultOptions();
  img = dmtxImage(image);


  dmtxImageSetProp(img, DmtxPropImageFlip, DmtxFlipNone);

  /* Initialize scan */
  dec = dmtxDecodeCreate(img, opt.shrinkMin);
  if(dec == NULL) {
    //CleanupMagick(&wand, DmtxFalse);
    //FatalError(EX_SOFTWARE, "decode create error");
  }

  err = SetDecodeOptions(dec, img, &opt);
  if(err != DmtxPass) {
    //CleanupMagick(&wand, DmtxFalse);
    //FatalError(EX_SOFTWARE, "decode option error");
  }

  /* Find and decode every barcode on page */
  pageScanCount = 0;
  for(;;) {
    /* Find next barcode region within image, but do not decode yet */
    if(opt.timeoutMS == DmtxUndefined)
        reg = dmtxRegionFindNext(dec, NULL);
    else
        reg = dmtxRegionFindNext(dec, &timeout);

    /* Finished file or ran out of time before finding another region */
    if(reg == NULL)
        break;

    /* Decode region based on requested barcode mode */
    if(opt.mosaic == DmtxTrue)
        msg = dmtxDecodeMosaicRegion(dec, reg, opt.correctionsMax);
    else
        msg = dmtxDecodeMatrixRegion(dec, reg, opt.correctionsMax);

    if(msg != NULL) {
        //PrintStats(dec, reg, msg, imgPageIndex, &opt);
        //PrintMessage(reg, msg, &opt);

        std::string decodedText = std::string((char*) msg->output);
        std::cout << ">" << decodedText << std::endl;
        codes.push_back(decodedText);
        /*
        for(i = 0; i < msg->codeSize; i++) {
          std::cout << "" << msg->code[i];
        } 
        */
        //std::cout << "" << msg->code[i] << std::endl;
        pageScanCount++;
        imgScanCount++;
        dmtxMessageDestroy(&msg);
    }
    dmtxRegionDestroy(&reg);
  }
  dmtxDecodeDestroy(&dec);
  dmtxImageDestroy(&img);
}



/**
 * Converts an OpenCV image to a DmtxImage.
 *
 * NOTE: The caller must detroy the image.
 */
DmtxImage* Image::dmtxImage(cv::Mat &image) {
    if (image.type() != CV_8UC1) {
        throw std::logic_error("invalid image type");// + image.type());
    }
    DmtxImage * dmtxImage = dmtxImageCreate( image.data, image.cols, image.rows, DmtxPack8bppK);
    dmtxImageSetProp(dmtxImage, DmtxPropRowPadBytes, image.step1() - image.cols);
    return dmtxImage;
}


void Image::barcode(){

  codes={};
  cv::Mat mat = largestSimpleContour(processImage);
  reducedImage = mat.clone();
  showImage(mat);
  std::vector<cv::Mat> candidates = findDMTXRectangles(mat);
  for(cv::Mat candidate : candidates) {
    datamatrix(candidate);
  }

  barcode_internal(processImage);

  sort( codes.begin(), codes.end() );
  codes.erase( unique( codes.begin(), codes.end() ), codes.end() );

  

  
}

cv::Mat Image::getImage(){
  cv::Mat res;
  cvtColor(reducedImage,res,CV_GRAY2BGR);
  return res;
}

void Image::save(std::string name,std::vector<int> params){
  const char* fnamec = name.c_str();
//  std::cout << fnamec << std::endl;
  cv::imwrite(fnamec,processImage,params);
}



bcResult Image::barcode_internal(cv::Mat &part) {
  _debugTime("start barcode_internal");

  bcResult res = {cv::Point(0,0),cv::Rect(0,0,1,1),std::string(""),std::string(""),false};
  std::string code="";
  if (part.channels()>1){
    throw std::runtime_error("Error: Image::barcode_internal not a gray image");
  }

cv::Mat grayo;
int i_bc_thres_stop=250;
int i_bc_thres_start=60;
int i_bc_thres_step=5;

for (int thres=i_bc_thres_stop; thres>=i_bc_thres_start ;thres-=i_bc_thres_step){

      cv::threshold(part,grayo,thres,255, CV_THRESH_BINARY );
      //showImage(grayo,100);

      /*
      cv::Size ksize(3,3);
      cv::GaussianBlur(gray, tgray, ksize, 0);
      */

      //cv::Mat grayo=part.clone();
      zbar::Image* _image;
      zbar::ImageScanner* _imageScanner;
      _image = new zbar::Image(grayo.cols, grayo.rows, "Y800", nullptr, 0);
      _imageScanner = new zbar::ImageScanner();
      _imageScanner->set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 1);

      
      _imageScanner->set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 1);
      _imageScanner->set_config(zbar::ZBAR_CODE128, zbar::ZBAR_CFG_ENABLE, 1);
      _imageScanner->set_config(zbar::ZBAR_CODE128, zbar::ZBAR_CFG_ADD_CHECK, 1);
      _imageScanner->set_config(zbar::ZBAR_CODE128, zbar::ZBAR_CFG_EMIT_CHECK, 0);


      _imageScanner->set_config(zbar::ZBAR_CODE39, zbar::ZBAR_CFG_ENABLE, 1);
      _imageScanner->set_config(zbar::ZBAR_CODE39, zbar::ZBAR_CFG_ADD_CHECK, 1);
      _imageScanner->set_config(zbar::ZBAR_CODE39, zbar::ZBAR_CFG_EMIT_CHECK, 0);


      _imageScanner->set_config(zbar::ZBAR_I25, zbar::ZBAR_CFG_ENABLE, 1);
      _imageScanner->set_config(zbar::ZBAR_I25, zbar::ZBAR_CFG_ADD_CHECK, 1);
      _imageScanner->set_config(zbar::ZBAR_I25, zbar::ZBAR_CFG_EMIT_CHECK, 0);

      /*
      _imageScanner->set_config(zbar::ZBAR_QRCODE, zbar::ZBAR_CFG_ENABLE, 1);
      _imageScanner->set_config(zbar::ZBAR_QRCODE, zbar::ZBAR_CFG_ADD_CHECK, 1);
      _imageScanner->set_config(zbar::ZBAR_QRCODE, zbar::ZBAR_CFG_EMIT_CHECK, 0);
      */
      
      _image->set_data((uchar *)grayo.data, grayo.cols * grayo.rows);
      
      _imageScanner->scan(*_image);
      for(zbar::Image::SymbolIterator symbol = _image->symbol_begin(); symbol != _image->symbol_end(); ++symbol) {
        if (showDebug){
          std::cout << "without thres Code " << symbol->get_data().c_str() << " Type " << symbol->get_type_name().c_str() << std::endl;
        }
        code = std::string(symbol->get_data().c_str());
        std::string type = std::string(symbol->get_type_name().c_str());
        codes.push_back(code);

      }
      }
  _debugTime("stop barcode_internal -----> "+res.code);
  return res;
}


std::vector<std::string> Image::getCodes(){
  return codes;
}