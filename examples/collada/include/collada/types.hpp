#ifndef __TYPES_HPP_INCLUDED__
#define __TYPES_HPP_INCLUDED__

#include <object.hpp>
#include <scoped_array.hpp>
#include <conststring.hpp>
#include <image.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace asset {



struct material {
  glm::vec4 ambient;
  glm::vec4 diffuse;
  glm::vec4	specular;
  glm::vec4 emission;
  float shininess;
};

struct light {
 glm::vec4 position;
 glm::vec4 ambient;
 glm::vec4 diffuse;
 glm::vec4 specular;
};

/*
constexpr const material DEFAULT_MATERIAL = {
	{0.2F, 0.2F, 0.2F, 1.0F},
	{0.8F, 0.8F, 0.8F, 1.0F},
	{0.0F, 0.0F, 0.0F, 1.0F},
	{0.0F, 0.0F, 0.0F, 1.0F},
	0.0F
};

constexpr const light DEFAULT_LIGHT = {
	{0.0F,0.0F,1.0F},
	{0.0F,0.0F,0.0F,1.0F},
	{1.0F,1.0F,1.0F,1.0F},
	{1.0F,1.0F,1.0F,1.0F}
};
*/


enum class primitive_type: uint32_t {
	point,
	line,
	triangle,
	polygon
};

struct face_normalized
{
	glm::vec3 position;
	glm::vec3 normal;
};

template<class _Iter>
_Iter store(const face_normalized& dst,_Iter& to)
{
//	static_assert( std::is_same<float, std::iterator_traits<_Iter>::value_type>::value, "Must be float iterator" );
	float *p = glm::value_ptr(dst.position);
	_Iter ret = std::copy( p, p+3, to);
	p = glm::value_ptr(dst.normal);
	return std::copy(p,p+3,ret);
}


struct face_textured
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texture_pos;
};

struct face_textured_normal_maped
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texture_pos;
	glm::vec3 tangent;
};

class mesh;
DECLARE_IPTR(mesh);

class mesh:public io::object
{
public:
private:
	io::scoped_arr<float> vertex_;
	io::scoped_arr<unsigned int> faces_;
	std::size_t mat_idx_;
};

class textured_mesh: public mesh
{
public:
};

class normal_mapped_mesh:public textured_mesh
{
};


} // namesapace asset


#endif // __TYPES_HPP_INCLUDED__
