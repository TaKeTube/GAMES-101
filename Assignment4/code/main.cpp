#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>

std::vector<cv::Point2f> control_points;
cv::Point2f p;
int dragged_flag = 0;

void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window);

float distance2(cv::Point2f a, cv::Point2f b){
    return (a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y);
}

void mouse_handler(int event, int x, int y, int flags, void *userdata) 
{
    if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < 4) 
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
        << y << ")" << '\n';
        control_points.emplace_back(x, y);
        return;
    }
    if (event == cv::EVENT_LBUTTONDOWN){
        p.x = x;
        p.y = y;
        for(int i = 0; i < control_points.size(); i++){
            if(distance2(p, control_points[i]) < 36.f){
                dragged_flag = i+1;
            }
        }
    }
    else if((event == cv::EVENT_MOUSEMOVE) && (flags & cv::EVENT_FLAG_LBUTTON) && dragged_flag){
        control_points[dragged_flag-1].x = x;
        control_points[dragged_flag-1].y = y;
    }
    else if(event == cv::EVENT_LBUTTONUP){
        dragged_flag == 0;
    }
    /*if ((event == cv::EVENT_MOUSEMOVE) && (flags & cv::EVENT_FLAG_LBUTTON)){
        p.x = x;
        p.y = y;
        for(int i = 0; i < control_points.size(); i++){
            if(distance2(p, control_points[i]) < 36.f){
                control_points[i].x = x;
                control_points[i].y = y;
            }
        }
    }*/
}


void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window) 
{
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.001) 
    {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                 3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t) 
{
    // TODO: Implement de Casteljau's algorithm
    int size = control_points.size();
    if(size <= 1){
        return cv::Point2f(control_points[0].x, control_points[0].y);
    }
    std::vector<cv::Point2f> temp;
    for(int i = 0; i < size - 1; i++){
        temp.push_back(t*control_points[i+1] + (1-t)*control_points[i]);
    }
    return recursive_bezier(temp, t);
}

void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window) 
{
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's 
    // recursive Bezier algorithm.
    for (double t = 0.0; t <= 1.0; t += 0.001){
        auto point = recursive_bezier(control_points, t);
        float c = 1.f, xp = point.x, yp = point.y;
        for(float x = xp - c; x < xp + c; x++){
            for(float y = yp - c; y < yp + c; y++){
                float d2 = (xp-x)*(xp-x) + (yp-y)*(yp-y);
                if(d2 <= c*c){
                    window.at<cv::Vec3b>(y, x)[1] = 255;
                }
            }
        }
    }
}

int main() 
{
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    //cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);
    int key = -1;
    while (key != 27) 
    {
        window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
        for (auto &point : control_points) 
        {
            cv::circle(window, point, 3, {255, 255, 255}, 3);
        }

        if (control_points.size() == 4) 
        {
            //naive_bezier(control_points, window);
            bezier(control_points, window);

            cv::imshow("Bezier Curve", window);
            //cv::imwrite("my_bezier_curve.png", window);
            key = cv::waitKey(1);
            //return 0;
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(1);
    }
    return 0;
}
