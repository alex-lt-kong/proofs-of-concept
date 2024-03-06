// https://docs.opencv.org/4.x/d0/dd4/tutorial_dnn_face.html

#include <iostream>
#include <signal.h>
#include <stdlib.h>

#include <opencv2/dnn.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>

using namespace cv;
using namespace std;

volatile sig_atomic_t e_flag = 0;

static void signal_handler(int signum) {
  char msg[] = "Signal [  ] caught\n";
  msg[8] = '0' + (char)(signum / 10);
  msg[9] = '0' + (char)(signum % 10);
  write(STDIN_FILENO, msg, strlen(msg));
  e_flag = 1;
}

void install_signal_handler() {
  // This design canNOT handle more than 99 signal types
  if (_NSIG > 99) {
    fprintf(stderr, "signal_handler() can't handle more than 99 signals\n");
    abort();
  }
  struct sigaction act;
  // Initialize the signal set to empty, similar to memset(0)
  if (sigemptyset(&act.sa_mask) == -1) {
    perror("sigemptyset()");
    abort();
  }
  act.sa_handler = signal_handler;
  /* SA_RESETHAND means we want our signal_handler() to intercept the signal
  once. If a signal is sent twice, the default signal handler will be used
  again. `man sigaction` describes more possible sa_flags. */
  act.sa_flags = SA_RESETHAND;
  // act.sa_flags = 0;
  if (sigaction(SIGINT, &act, 0) == -1) {
    perror("sigaction()");
    abort();
  }
}

static void visualize(Mat &input, int frame, Mat &faces, double fps,
                      int thickness = 2) {
  std::string fpsString = cv::format("FPS : %.2f", (float)fps);
  if (frame >= 0)
    cout << "Frame " << frame << ", ";
  cout << "FPS: " << fpsString << endl;
  for (int i = 0; i < faces.rows; i++) {
    // Print results
    cout << "Face " << i << ", top-left coordinates: (" << faces.at<float>(i, 0)
         << ", " << faces.at<float>(i, 1) << "), "
         << "box width: " << faces.at<float>(i, 2)
         << ", box height: " << faces.at<float>(i, 3) << ", "
         << "score: " << cv::format("%.2f", faces.at<float>(i, 14)) << endl;

    // Draw bounding box
    rectangle(input,
              Rect2i(int(faces.at<float>(i, 0)), int(faces.at<float>(i, 1)),
                     int(faces.at<float>(i, 2)), int(faces.at<float>(i, 3))),
              Scalar(0, 255, 0), thickness);
    // Draw landmarks
    circle(input,
           Point2i(int(faces.at<float>(i, 4)), int(faces.at<float>(i, 5))), 2,
           Scalar(255, 0, 0), thickness);
    circle(input,
           Point2i(int(faces.at<float>(i, 6)), int(faces.at<float>(i, 7))), 2,
           Scalar(0, 0, 255), thickness);
    circle(input,
           Point2i(int(faces.at<float>(i, 8)), int(faces.at<float>(i, 9))), 2,
           Scalar(0, 255, 0), thickness);
    circle(input,
           Point2i(int(faces.at<float>(i, 10)), int(faces.at<float>(i, 11))), 2,
           Scalar(255, 0, 255), thickness);
    circle(input,
           Point2i(int(faces.at<float>(i, 12)), int(faces.at<float>(i, 13))), 2,
           Scalar(0, 255, 255), thickness);
  }
  putText(input, fpsString, Point(0, 15), FONT_HERSHEY_SIMPLEX, 0.5,
          Scalar(0, 255, 0), 2);
}

int main(int argc, char **argv) {
  install_signal_handler();
  if (argc < 2 || argc > 3) {
    cerr << "Usage: " << argv[0] << " <inputVideoPath> [outputVideoPath]"
         << endl;
    return EXIT_FAILURE;
  }
  cout << "OpenCV's build information:\n" << cv::getBuildInformation() << "\n";

  /// Model parameters
  // the threshold to filter out bounding boxes of score smaller than the given
  // value
  float scoreThreshold = 0.9;
  // the threshold to suppress bounding boxes of IoU bigger than the given value
  float nmsThreshold = 0.3;
  // keep top K bboxes before NMS
  int topK = 5000;
  /// Model parameters

  // Initialize FaceDetectorYN
  Ptr<FaceDetectorYN> detector = FaceDetectorYN::create(
      "./face_detection_yunet_2023mar.onnx", "", Size(1920, 1080),
      scoreThreshold, nmsThreshold, topK);

  TickMeter tm;

  int frameWidth, frameHeight;
  VideoCapture capture;
  string inputVideo = argv[1];
  string outputVideo = "";
  if (argc == 3) {
    outputVideo = argv[2];
  }
  capture.open(samples::findFileOrKeep(inputVideo)); // keep GStreamer pipelines
  if (capture.isOpened()) {
    frameWidth = int(capture.get(CAP_PROP_FRAME_WIDTH));
    frameHeight = int(capture.get(CAP_PROP_FRAME_HEIGHT));
    cout << "Video " << inputVideo << ": width=" << frameWidth
         << ", height=" << frameHeight << endl;
  } else {
    cout << "Could not initialize video capturing: " << inputVideo << "\n";
    return 1;
  }

  detector->setInputSize(Size(frameWidth, frameHeight));
  VideoWriter writer;
  if (outputVideo.length() > 0) {
    writer = VideoWriter(outputVideo, VideoWriter::fourcc('M', 'P', '4', 'V'),
                         30, Size(frameWidth, frameHeight));
  }

  int nFrame = 0;
  while (!e_flag) {
    // Get frame
    Mat frame;
    if (!capture.read(frame)) {
      cerr << "Can't grab frame! Stop\n";
      break;
    }
    resize(frame, frame, Size(frameWidth, frameHeight));

    // Inference
    Mat faces;
    tm.start();
    detector->detect(frame, faces);
    tm.stop();

    Mat result = frame.clone();
    // Draw results on the input image
    visualize(result, nFrame, faces, tm.getFPS());

    // Visualize results
    imshow("Live", result);

    int key = waitKey(1);

    if (outputVideo.length() > 0)
      writer.write(result);

    ++nFrame;
  }

  if (outputVideo.length() > 0)
    writer.release();
  cout << "Processed " << nFrame << " frames" << endl;

  cout << "Done." << endl;
  return 0;
}
