#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include "cmd.h"

using namespace cv;

vector<Mat> loadVideo(VideoCapture cap, int frameSkip, bool noGUI, int w, int h) {
    vector<Mat> frames;
    int count = 0;
    while (cap.isOpened()) {
        Mat frame;
        cap >> frame;
        if (frame.empty()) {
            break;
        }
        resize(frame, frame, Size(w, h));
        count++;
        if (count % frameSkip == 0) {
            frames.push_back(frame);
        }
        if (!noGUI) {
            imshow("Video", frame);
            waitKey(1);
        }
    }
    return frames;
}

Mat stitchFrames(vector<Mat> frames, bool noGUI) {
    Mat stitched = frames[0];
    for (int i = 1; i < frames.size(); i++) {
        vconcat(stitched, frames[i], stitched);
    }
    if (!noGUI) {
        imshow("stitched", stitched);
        waitKey(100);
    }
    return stitched;
}

void writeProps(string name, int w, int h, char* type) {
    ofstream propsFile;
    propsFile.open("optifine/anim/" + name + "/" + name + ".properties");
    propsFile << "duration=1\n";
    propsFile << "w=" << w << "\n";
    propsFile << "h=" << h << "\n";
    propsFile << "x=0\n";
    propsFile << "y=0\n";
    propsFile << "from=./" << name << ".png\n";
    propsFile << "to=textures/" << type << ".png\n";
    propsFile.close();
}

int main(int argc, char* argv[]) {
    int frameSkip = getCmdIntOrDefault(argv, argv + argc, "-s", 1);
    bool noGUI = cmdOptionExists(argv, argv + argc, "--no-gui");
    char* filename = getCmdOption(argv, argv + argc, "-f");
    char* type = getCmdOption(argv, argv + argc, "--type");
    if (filename == 0) {
        cout << "No file specified!" << endl;
        return -1;
    }
    if (type == 0) {
        cout << "No type specified!" << endl;
        return -1;
    }
    string name = string(filename).substr(string(filename).find_last_of("/\\") + 1);
    name = name.substr(0, name.find_last_of("."));

    cout << "Loading... ";
    VideoCapture cap(filename);
    if (!cap.isOpened()) {
        cout << "Error opening video stream or file" << endl;
        return -1;
    }

    int w = getCmdIntOrDefault(argv, argv + argc, "-w", cap.get(CAP_PROP_FRAME_WIDTH));
    int h = getCmdIntOrDefault(argv, argv + argc, "-h", cap.get(CAP_PROP_FRAME_HEIGHT));

    vector<Mat> frames = loadVideo(cap, frameSkip, noGUI, w, h);
    cap.release();
    destroyAllWindows();
    cout << "DONE!\n";
    cout << "Got " << frames.size() << " frames!\n";

    cout << "Stitching... ";
    Mat stitched = stitchFrames(frames, noGUI);

    filesystem::create_directories("./optifine/anim/" + name);
    filesystem::create_directories("./textures/" + string(type).substr(0, string(type).find_last_of("/")));

    imwrite("optifine/anim/" + name + "/" + name + ".png", stitched);
    imwrite("textures/" + string(type) + ".png", frames[0]);

    writeProps(name, w, h, type);
    cout << "DONE!\n";
	return 0;
}