#include <geometry.hpp>
#include <pugixml.hpp>
#include <fileutility.hpp>
#include <formatting.hpp>
#include <functional>

using namespace std::placeholders;

bool check_node_name(pugi::xml_node const n, char const *name)
{
    return 0 == std::strcmp(n.name(), name);
}

bool check_attribute_name(pugi::xml_attribute const &a, char const *name)
{
    return 0 == std::strcmp(a.name(), name);
}

bool check_attribute_value(pugi::xml_attribute const &a, char const *name)
{
    return 0 == std::strcmp(a.value(), name);
}

void parse_vec3(std::string const &buf, double *v)
{
    std::stringstream sstr{buf};
    sstr >> v[0] >> v[1] >> v[2];
}

auto parse_angles(std::string const &str)
{
    double angles[3];
    parse_vec3(str, angles);
    for (std::size_t i{}; i < 3; ++i)
    {
        angles[i] = math::deg_to_rad(angles[i]);
    }
    math::quaternion q;
    math::quaternion::angles_to_quat(angles[0], angles[1], angles[2], q, math::RotationOrder::yxz);
    return q;
}

auto parse_origin(std::string const &str)
{
    math::vec3 v;
    parse_vec3(str, v.data());
    return v;
}

bool parse_notload(char const *str)
{
    return std::strcmp(str, "true") == 0;
}

void push_planes_pair(math::vec3 n, std::vector<geometry> &geometries)
{
    double s = n.length();
    n.normalize();
    geometries.push_back(geometry{.s = s, .n = n});
    geometries.push_back(geometry{.s = s, .n = -n});
}

void parse_plane(pugi::xml_node const &node, std::vector<geometry> &geometries, math::quaternion const &q)
{
    math::vec3 a, b, c, d;
    auto attr = node.find_attribute(std::bind(&check_attribute_name, _1, "vertex_A"));
    parse_vec3(attr.value(), a.data());
    attr = node.find_attribute(std::bind(&check_attribute_name, _1, "vertex_B"));
    parse_vec3(attr.value(), b.data());
    attr = node.find_attribute(std::bind(&check_attribute_name, _1, "vertex_C"));
    parse_vec3(attr.value(), c.data());
    attr = node.find_attribute(std::bind(&check_attribute_name, _1, "vertex_D"));
    parse_vec3(attr.value(), d.data());
    push_planes_pair(q.rotate(cross(b - a, d - a)), geometries);
}

void parse_cube(pugi::xml_node const &node, std::vector<geometry> &geometries, math::quaternion const &q)
{
    math::vec3 a, b, d, t;
    auto attr = node.find_attribute(std::bind(&check_attribute_name, _1, "vertex_A"));
    parse_vec3(attr.value(), a.data());
    attr = node.find_attribute(std::bind(&check_attribute_name, _1, "vertex_B"));
    parse_vec3(attr.value(), b.data());
    attr = node.find_attribute(std::bind(&check_attribute_name, _1, "vertex_D"));
    parse_vec3(attr.value(), d.data());
    attr = node.find_attribute(std::bind(&check_attribute_name, _1, "vertex_top"));
    parse_vec3(attr.value(), t.data());
    math::vec3 ba = b - a, da = d - a, ta = t - a;
    push_planes_pair(q.rotate(cross(ba, da)), geometries);
    push_planes_pair(q.rotate(cross(ba, ta)), geometries);
    push_planes_pair(q.rotate(cross(ta, da)), geometries);
}

void parse_node_primitive(pugi::xml_node const &node, std::vector<geometry> &geometries, math::quaternion q)
{
    auto attr = node.find_attribute(std::bind(&check_attribute_name, _1, "prim_type"));
    if (check_attribute_value(attr, "CUBE"))
    {
        parse_cube(node, geometries, q);
    }
    else if (check_attribute_value(attr, "PLANE"))
    {
        parse_plane(node, geometries, q);
    }
}

void parse_iss_node(pugi::xml_node const &node, std::vector<geometry> &geometries, math::quaternion q, math::vec3 dv)
{
    bool not_load{};
    for (auto attr = node.first_attribute(); attr; attr = attr.next_attribute())
    {
        if (check_attribute_name(attr, "angles_transform"))
        {
            auto tmp = parse_angles(attr.value());
            q = mul(q, tmp);
        }
        else if (check_attribute_name(attr, "origin"))
        {
            auto tmp = parse_origin(attr.value());
            dv += tmp;
        }
        else if (check_attribute_name(attr, "not_load"))
        {
            not_load = parse_notload(attr.value());
        }
    }
    if (not_load)
        return;
    for (auto child = node.first_child(); child; child = child.next_sibling())
    {
        if (check_node_name(child, "iss_node"))
        {
            parse_iss_node(child, geometries, q, dv);
        }
        else if (check_node_name(child, "node_primitive"))
        {
            parse_node_primitive(child, geometries, q);
        }
    }
}

std::vector<geometry> read_geometry_model_from_xml(fs::path const &filepath)
{
    auto fin = open_infile(filepath);
    pugi::xml_document doc;
    auto res = doc.load(fin);
    if (!res)
    {
        throw_runtime_error("Failed to read xml document %. Error status %. %", filepath, res.status, res.description());
    }
    auto root = doc.find_node(std::bind(&check_node_name, _1, "iss_model"));
    auto node = root.find_node(std::bind(&check_node_name, _1, "iss_node"));
    std::vector<geometry> geometries;
    parse_iss_node(node, geometries, math::quaternion{}, math::vec3{});
    for (auto &geom : geometries)
    {
        // перевод в метры
        geom.s *= 1e-6;
    }
    return geometries;
}