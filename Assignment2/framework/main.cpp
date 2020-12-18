// clang-format off
#include <iostream>
#include <opencv2/opencv.hpp>
#include "rasterizer.hpp"
#include "global.hpp"
#include "Triangle.hpp"

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1,0,0,-eye_pos[0],
                 0,1,0,-eye_pos[1],
                 0,0,1,-eye_pos[2],
                 0,0,0,1;

    view = translate*view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    //Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
    //return model;

    float angle_rad = rotation_angle / 180 * MY_PI;
    Eigen::Matrix4f rotate;
    rotate <<   cos(angle_rad), -sin(angle_rad),    0, 0,
                sin(angle_rad), cos(angle_rad),     0, 0,
                0,              0,                  1, 0,
                0,              0,                  0, 1;
    return rotate;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio, float zNear, float zFar)
{
    // TODO: Copy-paste your implementation from the previous assignment.
    
    //original version
    /*Eigen::Matrix4f perspective;
    Eigen::Matrix4f orthogonal;
    Eigen::Matrix4f translate;
    Eigen::Matrix4f scale;
    
    perspective <<  zNear,  0,      0,          0, 
                    0,      zNear,  0,          0, 
                    0,      0,      zNear+zFar, -zNear*zFar, 
                    0,      0,      1,          0;
    
    translate <<    1, 0, 0, 0, 
                    0, 1, 0, 0, 
                    0, 0, 1, -(zNear + zFar)*0.5,
                    0, 0, 0, 1;
    
    float inv_t = 1 / (zNear * tan(eye_fov / 180 * MY_PI * 0.5));
    float inv_r = inv_t * 1 / aspect_ratio;
    
    scale <<    inv_r,  0,      0,                  0, 
                0,      inv_t,  0,                  0, 
                0,      0,      2/(zNear - zFar),   0, 
                0,      0,      0,                  1;

    orthogonal = scale * translate;
    return orthogonal * perspective;*/

    //computing improved version
    Eigen::Matrix4f projection;
    float inv_tan = 1 / tan(eye_fov / 180 * MY_PI * 0.5);
    float k = 1 / (zNear-zFar);
    projection <<   inv_tan/aspect_ratio, 0,       0,              0,
                    0,                    inv_tan, 0,              0,
                    0,                    0,       (zNear+zFar)*k, 2*zFar*zNear*k,
                    0,                    0,       1,              0;
    return projection;

}

Eigen::Matrix4f get_rotation(Vector3f axis, float angle)
{
    float angle_rad = angle / 180 * MY_PI;
    float x = axis[0];
    float y = axis[1];
    float z = axis[2];
    
    //original version
    /*Eigen::Matrix4f N;
    Eigen::Matrix4f rotation;
    Eigen::Matrix<float, 4, 1> n;
    n << x, y, z, 0;
    N <<    0,  -z, y,  0,
            z,  0,  -x, 0,
            -y, x,  0,  0,
            0,  0,  0,  1;
    N = sin(angle_rad) * N;
    rotation = cos(angle_rad) * Eigen::Matrix4f::Identity() + (1 - cos(angle_rad)) * n * n.transpose() + N;
    rotation(3, 3) = 1;
    return rotation;*/

    //computing improved version
    float c = cos(angle_rad);
    float s = sin(angle_rad);
    float mc = 1 - cos(angle_rad);
    Eigen::Matrix4f R;
    R << c  +x*x*mc, -z*s+x*y*mc,  y*s+x*z*mc, 0,
         z*s+x*y*mc,  c  +y*y*mc, -x*s+y*z*mc, 0,
        -y*s+x*z*mc,  x*s+y*z*mc,  c  +z*z*mc, 0,
         0         ,  0         ,  0         , 1;
    return R;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc == 2)
    {
        command_line = true;
        filename = std::string(argv[1]);
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0,0,20};


    std::vector<Eigen::Vector3f> pos
            {
                    {2, 0, -2},
                    {0, 2, -2},
                    {-2, 0, -2},
                    {3.5, -1, -6},
                    {2.5, 1.5, -5},
                    {-1, 0.5, -3}
            };

    std::vector<Eigen::Vector3i> ind
            {
                    {0, 1, 2},
                    {3, 4, 5}
            };

    std::vector<Eigen::Vector3f> cols
            {
                    {217.0, 238.0, 185.0},
                    {217.0, 238.0, 185.0},
                    {217.0, 238.0, 185.0},
                    {185.0, 217.0, 238.0},
                    {185.0, 217.0, 238.0},
                    {185.0, 217.0, 238.0}
            };

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);
    auto col_id = r.load_colors(cols);

    int key = 0;
    int frame_count = 0;

    int animation = 0;

    //rotation axis
    Eigen::Vector3f axis = {1, -0.5, 0};

    if (command_line)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, col_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::cvtColor(image, image, cv::COLOR_RGB2BGR);

        cv::imwrite(filename, image);

        return 0;
    }

    while(key != 27)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        //r.set_model(get_model_matrix(angle));
        r.set_model(get_rotation(axis, angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, col_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::cvtColor(image, image, cv::COLOR_RGB2BGR);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        // super sample anti-aliasing
        if (key == 'z') {
            r.set_SSAA(1);
        }
        if (key == 'x') {
            r.set_SSAA(0);
        }

        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }

        if (key == 'w') {
            animation = 1;
        }
        else if (key == 's'){
            animation = -1;
        }
        else if (key == 'e'){
            animation = 0;
        }

        if(animation == 1){
            angle += 10;
        }
        else if (animation == -1){
            angle -= 10;
        }

    }

    return 0;
}
// clang-format on