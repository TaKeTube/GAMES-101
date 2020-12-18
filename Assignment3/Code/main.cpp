#include <iostream>
#include <opencv2/opencv.hpp>

#include "global.hpp"
#include "rasterizer.hpp"
#include "Triangle.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "OBJ_Loader.h"

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

Eigen::Matrix4f get_model_matrix(float angle)
{
    Eigen::Matrix4f rotation;
    angle = angle * MY_PI / 180.f;
    rotation << cos(angle), 0, sin(angle), 0,
                0, 1, 0, 0,
                -sin(angle), 0, cos(angle), 0,
                0, 0, 0, 1;

    Eigen::Matrix4f scale;
    scale << 2.5, 0, 0, 0,
              0, 2.5, 0, 0,
              0, 0, 2.5, 0,
              0, 0, 0, 1;

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1;

    return translate * rotation * scale;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio, float zNear, float zFar)
{
    // TODO: Use the same projection matrix from the previous assignments
    //computing improved version
    Eigen::Matrix4f projection;
    zNear = -zNear;
    zFar = -zFar;
    float inv_tan = 1 / tan(eye_fov / 180 * MY_PI * 0.5);
    float k = 1 / (zNear-zFar);
    projection <<   inv_tan/aspect_ratio, 0,       0,              0,
                    0,                    inv_tan, 0,              0,
                    0,                    0,       (zNear+zFar)*k, 2*zFar*zNear*k,
                    0,                    0,       1,              0;
    return projection;
}

Eigen::Vector3f vertex_shader(const vertex_shader_payload& payload)
{
    return payload.position;
}

Eigen::Vector3f normal_fragment_shader(const fragment_shader_payload& payload)
{
    Eigen::Vector3f return_color = (payload.normal.head<3>().normalized() + Eigen::Vector3f(1.0f, 1.0f, 1.0f)) / 2.f;
    Eigen::Vector3f result;
    result << return_color.x() * 255, return_color.y() * 255, return_color.z() * 255;
    return result;
}

static Eigen::Vector3f reflect(const Eigen::Vector3f& vec, const Eigen::Vector3f& axis)
{
    auto costheta = vec.dot(axis);
    return (2 * costheta * axis - vec).normalized();
}

struct light
{
    Eigen::Vector3f position;
    Eigen::Vector3f intensity;
};

Eigen::Vector3f texture_fragment_shader(const fragment_shader_payload& payload)
{
    Eigen::Vector3f return_color = {0, 0, 0};
    if (payload.texture)
    {
        // TODO: Get the texture value at the texture coordinates of the current fragment
        float u = payload.tex_coords[0];
        float v = payload.tex_coords[1];
        return_color = payload.texture->getColor(u, v);
    }
    Eigen::Vector3f texture_color;
    texture_color << return_color.x(), return_color.y(), return_color.z();

    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
    Eigen::Vector3f kd = texture_color / 255.f;
    Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);

    auto l1 = light{{20, 20, 20}, {500, 500, 500}};
    auto l2 = light{{-20, 20, 0}, {500, 500, 500}};

    std::vector<light> lights = {l1, l2};
    Eigen::Vector3f amb_light_intensity{10, 10, 10};
    Eigen::Vector3f eye_pos{0, 0, 10};

    float p = 150;

    Eigen::Vector3f color = texture_color;
    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal;

    Eigen::Vector3f result_color = {0, 0, 0};

    for (auto& light : lights)
    {
        // TODO: For each light source in the code, calculate what the *ambient*, *diffuse*, and *specular* 
        // components are. Then, accumulate that result on the *result_color* object.
        Eigen::Vector3f light_direction = (light.position - point).normalized();
        Eigen::Vector3f eye_direction = (eye_pos - point).normalized();
        Eigen::Vector3f intensity = light.intensity/((light.position - point).squaredNorm());
        // compute the ambient term
        Eigen::Vector3f ambient{ka[0]*amb_light_intensity[0], ka[1]*amb_light_intensity[1], ka[2]*amb_light_intensity[2]};
        // compute the diffuse term
        float diff_dot = std::max(0.f, normal.dot(light_direction));
        Eigen::Vector3f diffuse{kd[0]*intensity[0]*diff_dot, kd[1]*intensity[1]*diff_dot, kd[2]*intensity[2]*diff_dot};
        // compute the specular term
        Eigen::Vector3f half_v = (light_direction + eye_direction).normalized();
        float specular_dot = pow(std::max(0.f, normal.dot(half_v)),p);
        Eigen::Vector3f specular{ks[0]*intensity[0]*specular_dot, ks[1]*intensity[1]*specular_dot, ks[2]*intensity[2]*specular_dot};
        // compute the final color
        result_color += (ambient + diffuse + specular);
    }

    return result_color * 255.f;
}

Eigen::Vector3f phong_fragment_shader(const fragment_shader_payload& payload)
{
    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
    Eigen::Vector3f kd = payload.color;
    Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);

    auto l1 = light{{20, 20, 20}, {500, 500, 500}};
    auto l2 = light{{-20, 20, 0}, {500, 500, 500}};

    std::vector<light> lights = {l1, l2};
    Eigen::Vector3f amb_light_intensity{10, 10, 10};
    Eigen::Vector3f eye_pos{0, 0, 10};

    float p = 150;

    Eigen::Vector3f color = payload.color;
    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal;

    Eigen::Vector3f result_color = {0, 0, 0};
    for (auto& light : lights)
    {
        // TODO: For each light source in the code, calculate what the *ambient*, *diffuse*, and *specular* 
        // components are. Then, accumulate that result on the *result_color* object.
        Eigen::Vector3f light_direction = (light.position - point).normalized();
        Eigen::Vector3f eye_direction = (eye_pos - point).normalized();
        Eigen::Vector3f intensity = light.intensity/((light.position - point).squaredNorm());
        // compute the ambient term
        Eigen::Vector3f ambient{ka[0]*amb_light_intensity[0], ka[1]*amb_light_intensity[1], ka[2]*amb_light_intensity[2]};
        // compute the diffuse term
        float diff_dot = std::max(0.f, normal.dot(light_direction));
        Eigen::Vector3f diffuse{kd[0]*intensity[0]*diff_dot, kd[1]*intensity[1]*diff_dot, kd[2]*intensity[2]*diff_dot};
        // compute the specular term
        Eigen::Vector3f half_v = (light_direction + eye_direction).normalized();
        float specular_dot = pow(std::max(0.f, normal.dot(half_v)),p);
        Eigen::Vector3f specular{ks[0]*intensity[0]*specular_dot, ks[1]*intensity[1]*specular_dot, ks[2]*intensity[2]*specular_dot};
        // compute the final color
        result_color += (ambient + diffuse + specular);
    }
    return result_color * 255.f;
}



Eigen::Vector3f displacement_fragment_shader(const fragment_shader_payload& payload)
{
    
    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
    Eigen::Vector3f kd = payload.color;
    Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);

    auto l1 = light{{20, 20, 20}, {500, 500, 500}};
    auto l2 = light{{-20, 20, 0}, {500, 500, 500}};

    std::vector<light> lights = {l1, l2};
    Eigen::Vector3f amb_light_intensity{10, 10, 10};
    Eigen::Vector3f eye_pos{0, 0, 10};

    float p = 150;

    Eigen::Vector3f color = payload.color; 
    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal;

    float kh = 0.2, kn = 0.1;
    
    // TODO: Implement displacement mapping here
    // Let n = normal = (x, y, z)
    // Vector t = (x*y/sqrt(x*x+z*z),sqrt(x*x+z*z),z*y/sqrt(x*x+z*z))
    // Vector b = n cross product t
    float x = normal.x();
    float y = normal.y();
    float z = normal.z();
    Eigen::Vector3f tangent(-x*y/sqrt(x*x+z*z),sqrt(x*x+z*z),-z*y/sqrt(x*x+z*z));
    Eigen::Vector3f bitangent = normal.cross(tangent);
    // Matrix TBN = [t b n]
    Eigen::Matrix<float, 3, 3> TBN;
    TBN << tangent, bitangent, normal;
    // dU = kh * kn * (h(u+1/w,v)-h(u,v))
    // dV = kh * kn * (h(u,v+1/h)-h(u,v))
    float u = payload.tex_coords.x();
    float v = payload.tex_coords.y();
    auto texture = payload.texture;
    float width = texture->width;
    float height = texture->height;
    float dU = kh * kn * (texture->getColor(u+1/width,v).norm()-texture->getColor(u,v).norm());
    float dV = kh * kn * (texture->getColor(u,v+1/height).norm()-texture->getColor(u,v).norm());
    // Vector ln = (-dU, -dV, 1)
    Eigen::Vector3f ln(-dU, -dV, 1);
    // Position p = p + kn * n * h(u,v)
    // Normal n = normalize(TBN * ln)
    point = point + kn * normal * texture->getColor(u,v).norm();
    normal = (TBN * ln).normalized();

    Eigen::Vector3f result_color = {0, 0, 0};

    for (auto& light : lights)
    {
        // TODO: For each light source in the code, calculate what the *ambient*, *diffuse*, and *specular* 
        // components are. Then, accumulate that result on the *result_color* object.
                // components are. Then, accumulate that result on the *result_color* object.
        Eigen::Vector3f light_direction = (light.position - point).normalized();
        Eigen::Vector3f eye_direction = (eye_pos - point).normalized();
        Eigen::Vector3f intensity = light.intensity/((light.position - point).squaredNorm());
        // compute the ambient term
        Eigen::Vector3f ambient{ka[0]*amb_light_intensity[0], ka[1]*amb_light_intensity[1], ka[2]*amb_light_intensity[2]};
        // compute the diffuse term
        float diff_dot = std::max(0.f, normal.dot(light_direction));
        Eigen::Vector3f diffuse{kd[0]*intensity[0]*diff_dot, kd[1]*intensity[1]*diff_dot, kd[2]*intensity[2]*diff_dot};
        // compute the specular term
        Eigen::Vector3f half_v = (light_direction + eye_direction).normalized();
        float specular_dot = pow(std::max(0.f, normal.dot(half_v)),p);
        Eigen::Vector3f specular{ks[0]*intensity[0]*specular_dot, ks[1]*intensity[1]*specular_dot, ks[2]*intensity[2]*specular_dot};
        // compute the final color
        result_color += (ambient + diffuse + specular);
    }

    return result_color * 255.f;
}


Eigen::Vector3f bump_fragment_shader(const fragment_shader_payload& payload)
{
    
    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
    Eigen::Vector3f kd = payload.color;
    Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);

    auto l1 = light{{20, 20, 20}, {500, 500, 500}};
    auto l2 = light{{-20, 20, 0}, {500, 500, 500}};

    std::vector<light> lights = {l1, l2};
    Eigen::Vector3f amb_light_intensity{10, 10, 10};
    Eigen::Vector3f eye_pos{0, 0, 10};

    float p = 150;

    Eigen::Vector3f color = payload.color; 
    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal;

    float kh = 0.2, kn = 0.1;

    // TODO: Implement bump mapping here
    // Let n = normal = (x, y, z)
    // Vector t = (x*y/sqrt(x*x+z*z),sqrt(x*x+z*z),z*y/sqrt(x*x+z*z))
    // Vector b = n cross product t
    float x = normal.x();
    float y = normal.y();
    float z = normal.z();
    Eigen::Vector3f tangent(-x*y/sqrt(x*x+z*z),sqrt(x*x+z*z),-z*y/sqrt(x*x+z*z));
    Eigen::Vector3f bitangent = normal.cross(tangent);
    // Matrix TBN = [t b n]
    Eigen::Matrix<float, 3, 3> TBN;
    TBN << tangent, bitangent, normal;
    // dU = kh * kn * (h(u+1/w,v)-h(u,v))
    // dV = kh * kn * (h(u,v+1/h)-h(u,v))
    float u = payload.tex_coords.x();
    float v = payload.tex_coords.y();
    auto texture = payload.texture;
    float width = texture->width;
    float height = texture->height;
    float dU = kh * kn * (texture->getColor(u+1/width,v).norm()-texture->getColor(u,v).norm());
    float dV = kh * kn * (texture->getColor(u,v+1/height).norm()-texture->getColor(u,v).norm());
    // Vector ln = (-dU, -dV, 1)
    Eigen::Vector3f ln(-dU, -dV, 1);
    // Normal n = normalize(TBN * ln)
    normal = (TBN * ln).normalized();

    Eigen::Vector3f result_color = {0, 0, 0};
    result_color = normal;

    return result_color * 255.f;
}

Eigen::Vector3f normal_mapping_shader(const fragment_shader_payload& payload)
{
    
    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
    Eigen::Vector3f kd = payload.color;
    Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);

    auto l1 = light{{20, 20, 20}, {500, 500, 500}};
    auto l2 = light{{-20, 20, 0}, {500, 500, 500}};

    std::vector<light> lights = {l1, l2};
    Eigen::Vector3f amb_light_intensity{10, 10, 10};
    Eigen::Vector3f eye_pos{0, 0, 10};

    float p = 150;

    Eigen::Vector3f color = payload.color; 
    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal;

    float kh = 0.2, kn = 0.1;

    // Let n = normal = (x, y, z)
    // Vector t = (x*y/sqrt(x*x+z*z),sqrt(x*x+z*z),z*y/sqrt(x*x+z*z))
    // Vector b = n cross product t
    float x = normal.x();
    float y = normal.y();
    float z = normal.z();
    Eigen::Vector3f tangent(-x*y/sqrt(x*x+z*z),sqrt(x*x+z*z),-z*y/sqrt(x*x+z*z));
    Eigen::Vector3f bitangent = normal.cross(tangent);
    // Matrix TBN = [t b n]
    Eigen::Matrix<float, 3, 3> TBN;
    TBN << tangent, bitangent, normal;
    // Normal n = normalize(TBN * mapped_normal)
    float u = payload.tex_coords.x();
    float v = payload.tex_coords.y();
    auto texture = payload.texture;
    normal = (TBN * texture->getColor(u,v).normalized());

    Eigen::Vector3f result_color = {0, 0, 0};

    for (auto& light : lights)
    {
        // TODO: For each light source in the code, calculate what the *ambient*, *diffuse*, and *specular* 
        // components are. Then, accumulate that result on the *result_color* object.
                // components are. Then, accumulate that result on the *result_color* object.
        Eigen::Vector3f light_direction = (light.position - point).normalized();
        Eigen::Vector3f eye_direction = (eye_pos - point).normalized();
        Eigen::Vector3f intensity = light.intensity/((light.position - point).squaredNorm());
        // compute the ambient term
        Eigen::Vector3f ambient{ka[0]*amb_light_intensity[0], ka[1]*amb_light_intensity[1], ka[2]*amb_light_intensity[2]};
        // compute the diffuse term
        float diff_dot = std::max(0.f, normal.dot(light_direction));
        Eigen::Vector3f diffuse{kd[0]*intensity[0]*diff_dot, kd[1]*intensity[1]*diff_dot, kd[2]*intensity[2]*diff_dot};
        // compute the specular term
        Eigen::Vector3f half_v = (light_direction + eye_direction).normalized();
        float specular_dot = pow(std::max(0.f, normal.dot(half_v)),p);
        Eigen::Vector3f specular{ks[0]*intensity[0]*specular_dot, ks[1]*intensity[1]*specular_dot, ks[2]*intensity[2]*specular_dot};
        // compute the final color
        result_color += (ambient + diffuse + specular);
    }

    return result_color * 255.f;
}

Eigen::Vector3f my_shader_1(const fragment_shader_payload& payload)
{
    
    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
    Eigen::Vector3f kd = payload.color;
    Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);

    auto l1 = light{{20, 20, 20}, {500, 500, 500}};
    auto l2 = light{{-20, 20, 0}, {500, 500, 500}};

    std::vector<light> lights = {l1, l2};
    Eigen::Vector3f amb_light_intensity{10, 10, 10};
    Eigen::Vector3f eye_pos{0, 0, 10};

    float p = 150;

    Eigen::Vector3f color = payload.color; 
    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal;

    float kh = 0.2, kn = 0.1;
    
    float x = normal.x();
    float y = normal.y();
    float z = normal.z();
    Eigen::Vector3f tangent(-x*y/sqrt(x*x+z*z),sqrt(x*x+z*z),-z*y/sqrt(x*x+z*z));
    Eigen::Vector3f bitangent = normal.cross(tangent);
    Eigen::Matrix<float, 3, 3> TBN;
    TBN << tangent, bitangent, normal;
    float u = payload.tex_coords.x();
    float v = payload.tex_coords.y();
    auto texture = payload.texture;
    float width = texture->width;
    float height = texture->height;
    float dU = kh * kn * (texture->getColor(u+1/width,v).norm()-texture->getColor(u,v).norm());
    float dV = kh * kn * (texture->getColor(u,v+1/height).norm()-texture->getColor(u,v).norm());
    Eigen::Vector3f ln(-dU, -dV, 1);
    point = point + 0.03 * normal * texture->getColor(u,v).norm();
    normal = (TBN * ln).normalized();

    Eigen::Vector3f result_color = {0, 0, 0};

    for (auto& light : lights)
    {
        // TODO: For each light source in the code, calculate what the *ambient*, *diffuse*, and *specular* 
        // components are. Then, accumulate that result on the *result_color* object.
                // components are. Then, accumulate that result on the *result_color* object.
        Eigen::Vector3f light_direction = (light.position - point).normalized();
        Eigen::Vector3f eye_direction = (eye_pos - point).normalized();
        Eigen::Vector3f intensity = light.intensity/((light.position - point).squaredNorm());
        // compute the ambient term
        Eigen::Vector3f ambient{ka[0]*amb_light_intensity[0], ka[1]*amb_light_intensity[1], ka[2]*amb_light_intensity[2]};
        // compute the diffuse term
        float diff_dot = std::max(0.f, normal.dot(light_direction));
        Eigen::Vector3f diffuse{kd[0]*intensity[0]*diff_dot, kd[1]*intensity[1]*diff_dot, kd[2]*intensity[2]*diff_dot};
        // compute the specular term
        Eigen::Vector3f half_v = (light_direction + eye_direction).normalized();
        float specular_dot = pow(std::max(0.f, normal.dot(half_v)),p);
        Eigen::Vector3f specular{ks[0]*intensity[0]*specular_dot, ks[1]*intensity[1]*specular_dot, ks[2]*intensity[2]*specular_dot};
        // compute the final color
        result_color += (ambient + diffuse + specular);
    }

    return result_color * 255.f;
}

Eigen::Vector3f my_shader_2(const fragment_shader_payload& payload)
{
    Eigen::Vector3f return_color = {0, 0, 0};
    if (payload.texture)
    {
        float u = payload.tex_coords[0];
        float v = payload.tex_coords[1];
        return_color = payload.texture->getColor(u, v);
    }
    Eigen::Vector3f texture_color;
    texture_color << return_color.x(), return_color.y(), return_color.z();

    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
    Eigen::Vector3f kd = texture_color / 255.f;
    Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);

    auto l1 = light{{20, 20, 20}, {500, 500, 500}};
    auto l2 = light{{-20, 20, 0}, {500, 500, 500}};

    std::vector<light> lights = {l1, l2};
    Eigen::Vector3f amb_light_intensity{10, 10, 10};
    Eigen::Vector3f eye_pos{0, 0, 10};

    float p = 150;

    Eigen::Vector3f color = texture_color;
    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal;

    Eigen::Vector3f result_color = {0, 0, 0};

    Eigen::Vector3f eye_direction = (eye_pos - point).normalized();

    if(eye_direction.dot(normal) < 0.3 && eye_direction.dot(normal) > -0.3){
        float k = 10 * abs(eye_direction.dot(normal));
        result_color = {155.f + k, 155.f + k, 155.f + k};
        return result_color;
    }

    for (auto& light : lights)
    {
        // TODO: For each light source in the code, calculate what the *ambient*, *diffuse*, and *specular* 
        // components are. Then, accumulate that result on the *result_color* object.
        Eigen::Vector3f light_direction = (light.position - point).normalized();
        Eigen::Vector3f intensity = light.intensity/((light.position - point).squaredNorm());
        // compute the ambient term
        Eigen::Vector3f ambient{ka[0]*amb_light_intensity[0], ka[1]*amb_light_intensity[1], ka[2]*amb_light_intensity[2]};
        // compute the diffuse term
        float diff_dot = std::max(0.f, normal.dot(light_direction));
        Eigen::Vector3f diffuse{kd[0]*intensity[0]*diff_dot, kd[1]*intensity[1]*diff_dot, kd[2]*intensity[2]*diff_dot};
        // compute the specular term
        Eigen::Vector3f half_v = (light_direction + eye_direction).normalized();
        float specular_dot = pow(std::max(0.f, normal.dot(half_v)),p);
        Eigen::Vector3f specular{ks[0]*intensity[0]*specular_dot, ks[1]*intensity[1]*specular_dot, ks[2]*intensity[2]*specular_dot};
        // compute the final color
        result_color += (ambient + diffuse + specular);
    }

    return result_color * 255.f;
}

int main(int argc, const char** argv)
{
    std::vector<Triangle*> TriangleList;

    float angle = 140.0;
    bool command_line = false;

    std::string filename = "output.png";
    objl::Loader Loader;
    std::string obj_path = "../models/spot/";

    // Load .obj File
    bool loadout = Loader.LoadFile("../models/spot/spot_triangulated_good.obj");
    for(auto mesh:Loader.LoadedMeshes)
    {
        for(int i=0;i<mesh.Vertices.size();i+=3)
        {
            Triangle* t = new Triangle();
            // set triangle's vertex, normal, texture coordinate
            for(int j=0;j<3;j++)
            {
                t->setVertex(j,Vector4f(mesh.Vertices[i+j].Position.X,mesh.Vertices[i+j].Position.Y,mesh.Vertices[i+j].Position.Z,1.0));
                t->setNormal(j,Vector3f(mesh.Vertices[i+j].Normal.X,mesh.Vertices[i+j].Normal.Y,mesh.Vertices[i+j].Normal.Z));
                t->setTexCoord(j,Vector2f(mesh.Vertices[i+j].TextureCoordinate.X, mesh.Vertices[i+j].TextureCoordinate.Y));
            }
            // push the triangle into the triangle list
            TriangleList.push_back(t);
        }
    }

    rst::rasterizer r(700, 700);

    auto texture_path = "hmap.jpg";
    r.set_texture(Texture(obj_path + texture_path));

    // like a function pointer
    // std::function<ReturnType(ParameterType)> Name
    //std::function<Eigen::Vector3f(fragment_shader_payload)> active_shader = phong_fragment_shader;
    std::function<Eigen::Vector3f(fragment_shader_payload)> active_shader = my_shader_2;
    if (argc >= 2)
    {
        command_line = true;
        filename = std::string(argv[1]);

        if (argc == 3 && std::string(argv[2]) == "texture")
        {
            std::cout << "Rasterizing using the texture shader\n";
            active_shader = texture_fragment_shader;
            texture_path = "spot_texture.png";
            r.set_texture(Texture(obj_path + texture_path));
        }
        else if (argc == 3 && std::string(argv[2]) == "normal")
        {
            std::cout << "Rasterizing using the normal shader\n";
            active_shader = normal_fragment_shader;
        }
        else if (argc == 3 && std::string(argv[2]) == "phong")
        {
            std::cout << "Rasterizing using the phong shader\n";
            active_shader = phong_fragment_shader;
        }
        else if (argc == 3 && std::string(argv[2]) == "bump")
        {
            std::cout << "Rasterizing using the bump shader\n";
            active_shader = bump_fragment_shader;
        }
        else if (argc == 3 && std::string(argv[2]) == "displacement")
        {
            std::cout << "Rasterizing using the bump shader\n";
            active_shader = displacement_fragment_shader;
        }
        else if (argc == 3 && std::string(argv[2]) == "normal_map")
        {
            std::cout << "Rasterizing using the normal mapping shader\n";
            active_shader = normal_mapping_shader;
        }
        else if (argc == 3 && std::string(argv[2]) == "myshader1")
        {
            std::cout << "Rasterizing using my shader 1\n";
            active_shader = my_shader_1;
        }
        else if (argc == 3 && std::string(argv[2]) == "myshader2")
        {
            std::cout << "Rasterizing using my shader 2\n";
            texture_path = "spot_texture.png";
            active_shader = my_shader_2;
            r.set_texture(Texture(obj_path + texture_path));
        }
    }

    Eigen::Vector3f eye_pos = {0,0,10};

    r.set_vertex_shader(vertex_shader);
    r.set_fragment_shader(active_shader);

    int key = 0;
    int frame_count = 0;

    if (command_line)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);
        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45.0, 1, 0.1, 50));

        r.draw(TriangleList);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::cvtColor(image, image, cv::COLOR_RGB2BGR);

        cv::imwrite(filename, image);

        return 0;
    }

    while(key != 27)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45.0, 1, 0.1, 50));

        //r.draw(pos_id, ind_id, col_id, rst::Primitive::Triangle);
        r.draw(TriangleList);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::cvtColor(image, image, cv::COLOR_RGB2BGR);

        cv::imshow("image", image);
        cv::imwrite(filename, image);
        key = cv::waitKey(10);

        if (key == 'a' )
        {
            angle -= 0.1;
        }
        else if (key == 'd')
        {
            angle += 0.1;
        }
        angle += 2;

    }
    return 0;
}
