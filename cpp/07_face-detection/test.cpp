#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect.hpp>

#include <iostream>

using namespace cv;
using namespace std;

static
void visualize(Mat& input, int frame, Mat& faces, double fps, int thickness = 2)
{
    std::string fpsString = cv::format("FPS : %.2f", (float)fps);
    if (frame >= 0)
        cout << "Frame " << frame << ", ";
    cout << "FPS: " << fpsString << endl;
    for (int i = 0; i < faces.rows; i++)
    {
        // Print results
        cout << "Face " << i
             << ", top-left coordinates: (" << faces.at<float>(i, 0) << ", " << faces.at<float>(i, 1) << "), "
             << "box width: " << faces.at<float>(i, 2)  << ", box height: " << faces.at<float>(i, 3) << ", "
             << "score: " << cv::format("%.2f", faces.at<float>(i, 14))
             << endl;

        // Draw bounding box
        rectangle(input, Rect2i(int(faces.at<float>(i, 0)), int(faces.at<float>(i, 1)), int(faces.at<float>(i, 2)), int(faces.at<float>(i, 3))), Scalar(0, 255, 0), thickness);
        // Draw landmarks
        circle(input, Point2i(int(faces.at<float>(i, 4)), int(faces.at<float>(i, 5))), 2, Scalar(255, 0, 0), thickness);
        circle(input, Point2i(int(faces.at<float>(i, 6)), int(faces.at<float>(i, 7))), 2, Scalar(0, 0, 255), thickness);
        circle(input, Point2i(int(faces.at<float>(i, 8)), int(faces.at<float>(i, 9))), 2, Scalar(0, 255, 0), thickness);
        circle(input, Point2i(int(faces.at<float>(i, 10)), int(faces.at<float>(i, 11))), 2, Scalar(255, 0, 255), thickness);
        circle(input, Point2i(int(faces.at<float>(i, 12)), int(faces.at<float>(i, 13))), 2, Scalar(0, 255, 255), thickness);
    }
    putText(input, fpsString, Point(0, 15), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 2);
}

int main(int argc, char** argv)
{
    cout << cv::getBuildInformation() << "\n";
    CommandLineParser parser(argc, argv,
        "{help  h           |            | Print this message}"
        "{image1 i1         |            | Path to the input image1. Omit for detecting through VideoCapture}"
        "{image2 i2         |            | Path to the input image2. When image1 and image2 parameters given then the program try to find a face on both images and runs face recognition algorithm}"
        "{video v           | 0          | Path to the input video}"
        "{scale sc          | 1.0        | Scale factor used to resize input video frames}"
        "{fd_model fd       | face_detection_yunet_2021dec.onnx| Path to the model. Download yunet.onnx in https://github.com/opencv/opencv_zoo/tree/master/models/face_detection_yunet}"
        "{fr_model fr       | face_recognition_sface_2021dec.onnx | Path to the face recognition model. Download the model at https://github.com/opencv/opencv_zoo/tree/master/models/face_recognition_sface}"
        "{score_threshold   | 0.9        | Filter out faces of score < score_threshold}"
        "{nms_threshold     | 0.3        | Suppress bounding boxes of iou >= nms_threshold}"
        "{top_k             | 5000       | Keep top_k bounding boxes before NMS}"
        "{save s            | false      | Set true to save results. This flag is invalid when using camera}"
    );
    if (parser.has("help"))
    {
        parser.printMessage();
        return 0;
    }

    String fd_modelPath = parser.get<String>("fd_model");
    String fr_modelPath = parser.get<String>("fr_model");
    cout << "fd_modelPath: " << fd_modelPath << endl;
    cout << "fr_modelPath: " << fr_modelPath << endl;

    float scoreThreshold = parser.get<float>("score_threshold");
    float nmsThreshold = parser.get<float>("nms_threshold");
    int topK = parser.get<int>("top_k");

    bool save = parser.get<bool>("save");
    float scale = parser.get<float>("scale");

    double cosine_similar_thresh = 0.363;
    double l2norm_similar_thresh = 1.128;

    //! [initialize_FaceDetectorYN]
    // Initialize FaceDetectorYN
    // 
    // Ptr<FaceDetectorYN> detector = FaceDetectorYN::create(fd_modelPath, "", Size(320, 320), scoreThreshold, nmsThreshold, topK);
    Ptr<FaceDetectorYN> detector = FaceDetectorYN::create(
        "./face_detection_yunet_2023mar.onnx", 
        "", Size(1920, 1080), scoreThreshold, nmsThreshold, topK);
    //! [initialize_FaceDetectorYN]

    TickMeter tm;

  
    int frameWidth, frameHeight;
    VideoCapture capture;
    std::string video = parser.get<string>("video");
    video = "/tmp/20240303-205918.mp4";
    if (video.size() == 1 && isdigit(video[0]))
        capture.open(parser.get<int>("video"));
    else
        capture.open(samples::findFileOrKeep(video));  // keep GStreamer pipelines
    if (capture.isOpened())
    {
        frameWidth = int(capture.get(CAP_PROP_FRAME_WIDTH) * scale);
        frameHeight = int(capture.get(CAP_PROP_FRAME_HEIGHT) * scale);
        cout << "Video " << video
            << ": width=" << frameWidth
            << ", height=" << frameHeight
            << endl;
    }
    else
    {
        cout << "Could not initialize video capturing: " << video << "\n";
        return 1;
    }

    detector->setInputSize(Size(frameWidth, frameHeight));
    cv::VideoWriter writer(
        "output.mp4", cv::VideoWriter::fourcc('M', 'P', '4', 'V'), 30, cv::Size(frameWidth, frameHeight)
    );
    cout << "Press 'SPACE' to save frame, any other key to exit..." << endl;
    int nFrame = 0;
    for (;;)
    {
        // Get frame
        Mat frame;
        if (!capture.read(frame))
        {
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
        bool saveFrame = save;
        if (key == ' ')
        {
            saveFrame = true;
            key = 0;  // handled
        }

        if (saveFrame)
        {
            std::string frame_name = cv::format("frame_%05d.png", nFrame);
            std::string result_name = cv::format("result_%05d.jpg", nFrame);
            cout << "Saving '" << frame_name << "' and '" << result_name << "' ...\n";
            imwrite(frame_name, frame);
            imwrite(result_name, result);
        }
        writer.write(result);

        ++nFrame;

        if (key > 0 || nFrame > 400)
            break;
    }
    writer.release();
    cout << "Processed " << nFrame << " frames" << endl;
    
    cout << "Done." << endl;
    return 0;
}