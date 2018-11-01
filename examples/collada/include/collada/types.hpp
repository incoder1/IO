#ifndef __TYPES_HPP_INCLUDED__
#define __TYPES_HPP_INCLUDED__

#include <conststring.hpp>

namespace collada {

struct alignas(1) vec3 {
	float x;
	float y;
	float z;
};

struct alignas(1) color4 {
	float r;
	float g;
	float b;
	float a;
};


struct material {
  color4 ambient;
  color4 diffuse;
  color4 specular;
  color4 emission;
  float shininess;
};

struct light {
 vec3 position;
 color4 ambient;
 color4 diffuse;
 color4 specular;
};


constexpr const material DEFAULT_MATERIAL = {
	{0.2F, 0.2F, 0.2F, 1.0F},
	{0.8F, 0.8F, 0.8F, 1.0F},
	{0.0F, 0.0F, 0.0F, 1.0F},
	0.001F
};

constexpr const light DEFAULT_LIGHT = {
	{0.0F,0.0F,1.0F},
	{0.0F,0.0F,0.0F,1.0F},
	{1.0F,1.0F,1.0F,1.0F},
	{1.0F,1.0F,1.0F,1.0F}
};

struct asset {

};

struct camera {
    io::const_string id;
    io::const_string name;
};


} // namesapace collada


#endif // __TYPES_HPP_INCLUDED__
