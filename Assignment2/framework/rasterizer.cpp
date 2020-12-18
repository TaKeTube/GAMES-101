// clang-format off
//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>


rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions)
{
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols)
{
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f)
{
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}

/*
static bool insideTriangle(int x, int y, const Vector3f* _v)
{   
    // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]
    float ab[2] = {_v[0].x()-_v[1].x(), _v[0].y()-_v[1].y()};
    float bc[2] = {_v[1].x()-_v[2].x(), _v[1].y()-_v[2].y()};
    float ca[2] = {_v[2].x()-_v[0].x(), _v[2].y()-_v[0].y()};
    float ap[2] = {x-_v[1].x(), y-_v[1].y()};
    float bp[2] = {x-_v[2].x(), y-_v[2].y()};
    float cp[2] = {x-_v[0].x(), y-_v[0].y()};
    float abxap = ab[0]*ap[1] - ab[1]*ap[0];
    float bcxbp = bc[0]*bp[1] - bc[1]*bp[0];
    float caxcp = ca[0]*cp[1] - ca[1]*cp[0];
    if(((abxap<=0)&&(bcxbp<=0)&&(caxcp<=0))||((abxap>0)&&(bcxbp>0)&&(caxcp>0))){
        // debug code
        //std::cout << _v[0].x() << ' ' << _v[0].y() << std::endl;
        //std::cout << _v[1].x() << ' ' << _v[1].y() << std::endl;
        //std::cout << _v[2].x() << ' ' << _v[2].y() << std::endl;
        //std::cout << x << ' ' << y << std::endl;
        return 1;
    }else{
        return 0;
    }
}
*/

static bool insideTriangle(float x, float y, const Vector3f* _v)
{
    float ab[2] = {_v[0].x()-_v[1].x(), _v[0].y()-_v[1].y()};
    float bc[2] = {_v[1].x()-_v[2].x(), _v[1].y()-_v[2].y()};
    float ca[2] = {_v[2].x()-_v[0].x(), _v[2].y()-_v[0].y()};
    float ap[2] = {x-_v[1].x(), y-_v[1].y()};
    float bp[2] = {x-_v[2].x(), y-_v[2].y()};
    float cp[2] = {x-_v[0].x(), y-_v[0].y()};
    float abxap = ab[0]*ap[1] - ab[1]*ap[0];
    float bcxbp = bc[0]*bp[1] - bc[1]*bp[0];
    float caxcp = ca[0]*cp[1] - ca[1]*cp[0];
    if(((abxap<=0)&&(bcxbp<=0)&&(caxcp<=0))||((abxap>0)&&(bcxbp>0)&&(caxcp>0))){
        return 1;
    }else{
        return 0;
    }
}

static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f* v)
{
    float c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
    float c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
    float c3 = (x*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*y + v[0].x()*v[1].y() - v[1].x()*v[0].y()) / (v[2].x()*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*v[2].y() + v[0].x()*v[1].y() - v[1].x()*v[0].y());
    return {c1,c2,c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type)
{
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];
    auto& col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto& i : ind)
    {
        Triangle t;
        Eigen::Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)
        };
        //Homogeneous division
        for (auto& vec : v) {
            vec /= vec.w();
        }
        //Viewport transformation
        for (auto & vert : v)
        {
            vert.x() = 0.5*width*(vert.x()+1.0);
            vert.y() = 0.5*height*(vert.y()+1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i)
        {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t);
    }
}

//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    auto v = t.toVector4();
    // TODO : Find out the bounding box of current triangle.
    // iterate through the pixel and find if the current pixel is inside the triangle
    int y_max = std::min((int)ceil(std::max(std::max(v[0].y(),v[1].y()),v[2].y())), height);
    int y_min = std::max((int)floor(std::min(std::min(v[0].y(),v[1].y()),v[2].y())), 0);
    int x_max = std::min((int)ceil(std::max(std::max(v[0].x(),v[1].x()),v[2].x())), width);
    int x_min = std::max((int)floor(std::min(std::min(v[0].x(),v[1].x()),v[2].x())), 0);
    
    /* test for the insideTriangle() function
    Eigen::Vector3f t1 = {0, 0, 0};
    Eigen::Vector3f t2 = {2, 0, 0};
    Eigen::Vector3f t3 = {0, 2, 0};
    Eigen::Vector3f test[3] = {t1, t2, t3};
    std::cout<< insideTriangle(0.5, 1, test) <<std::endl;
    std::cout<< insideTriangle(2, 1, test) <<std::endl;
    std::cout<< insideTriangle(-2, 1, test) <<std::endl;
    std::cout<< insideTriangle(1, -1, test) <<std::endl;
    */
    
    if(SSAA){
        // super sample anti-aliasing
        int flag;
        for(int x = x_min; x <= x_max; x++){
            for(int y = y_min; y <= y_max; y++){
                flag = 0;
                int ind_a = (height*2-1-y*2)*width*2 + x*2;
                int ind_b = (height*2-1-y*2)*width*2 + x*2 + 1;
                int ind_c = (height*2-2-y*2)*width*2 + x*2;
                int ind_d = (height*2-2-y*2)*width*2 + x*2 + 1;
                if(insideTriangle((float)x, (float)y, t.v)){
                    auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
                    float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                    float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                    z_interpolated *= w_reciprocal;
                    if(sample_depth_buf[ind_a] > z_interpolated){
                        flag = 1;
                        sample_frame_buf[ind_a] = t.getColor();
                        sample_depth_buf[ind_a] = z_interpolated;
                    }
                }
                if(insideTriangle((float)x+0.5, (float)y, t.v)){
                    auto[alpha, beta, gamma] = computeBarycentric2D(x+0.5, y, t.v);
                    float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                    float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                    z_interpolated *= w_reciprocal;
                    if(sample_depth_buf[ind_b] > z_interpolated){
                        flag = 1;
                        sample_frame_buf[ind_b] = t.getColor();
                        sample_depth_buf[ind_b] = z_interpolated;
                    }
                }
                if(insideTriangle((float)x, (float)y+0.5, t.v)){
                    auto[alpha, beta, gamma] = computeBarycentric2D(x, y+0.5, t.v);
                    float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                    float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                    z_interpolated *= w_reciprocal;
                    if(sample_depth_buf[ind_c] > z_interpolated){
                        flag = 1;
                        sample_frame_buf[ind_c] = t.getColor();
                        sample_depth_buf[ind_c] = z_interpolated;
                    }
                }
                if(insideTriangle((float)x+0.5, (float)y+0.5, t.v)){
                    auto[alpha, beta, gamma] = computeBarycentric2D(x+0.5, y+0.5, t.v);
                    float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                    float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                    z_interpolated *= w_reciprocal;
                    if(sample_depth_buf[ind_d] > z_interpolated){
                        flag = 1;
                        sample_frame_buf[ind_d] = t.getColor();
                        sample_depth_buf[ind_d] = z_interpolated;
                    }
                }
                if(flag){
                    int ind = get_index(x,y);
                    frame_buf[ind] = (sample_frame_buf[ind_a]+sample_frame_buf[ind_b]+sample_frame_buf[ind_c]+sample_frame_buf[ind_d])*0.25;
                }
            }
        }
    }else{
        for(int x = x_min; x <= x_max; x++){
            for(int y = y_min; y <= y_max; y++){
                if(insideTriangle(x, y, t.v)){
                    // If so, use the following code to get the interpolated z value.
                    //std::cout << x << y << std::endl;
                    auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
                    float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                    float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                    z_interpolated *= w_reciprocal;
                    // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
                    int ind = get_index(x,y);
                    if(depth_buf[ind] > z_interpolated){
                        frame_buf[ind] = t.getColor();
                        depth_buf[ind] = z_interpolated;
                    }
                }
            }
        }
    }
}

void rst::rasterizer::set_model(const Eigen::Matrix4f& m)
{
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f& v)
{
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f& p)
{
    projection = p;
}

void rst::rasterizer::set_SSAA(int condition){
    SSAA = condition;
}

void rst::rasterizer::clear(rst::Buffers buff)
{
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        if(SSAA){
            std::fill(sample_depth_buf.begin(), sample_depth_buf.end(), std::numeric_limits<float>::infinity());
        }else{
            std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
        }
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(w * h);
    depth_buf.resize(w * h);
    sample_frame_buf.resize(4 * w * h);
    sample_depth_buf.resize(4 * w * h);
}

int rst::rasterizer::get_index(int x, int y)
{
    return (height-1-y)*width + x;
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height-1-point.y())*width + point.x();
    frame_buf[ind] = color;

}

// clang-format on