#include "stdafx.hpp"

#include "view.hpp"
#include "image.hpp"
#include "files.hpp"

#include "model.hpp"
#include "mesh.hpp"

#include "parser.hpp"

#include <iostream>

static const float COLORED_QUBE_VERTEX[216] = {
	// Coordinate  | Color | Normal

	// Top Quad
	1.0F, 1.0F,-1.0F,	0.5F, 0.5f, 0.5f,	0.0F, 1.0F, 0.0F,
	-1.0F, 1.0F,-1.0F,	0.5f, 0.5f, 0.5f,	0.0F, 1.0F, 0.0F,
	-1.0F, 1.0F, 1.0F,	0.5f, 0.5f, 0.5f,	0.0F, 1.0F, 0.0F,
	1.0F, 1.0F, 1.0F,	0.5f, 0.5f, 0.5f,	0.0F, 1.0F, 0.0F,

	// Bottom Quad
	1.0F,-1.0F, 1.0F,	0.0F, 1.0F, 0.0F,	 0.0F,-1.0F, 0.0F,
	-1.0F,-1.0F, 1.0F,	0.0F, 1.0F,0.0F,	 0.0F,-1.0F, 0.0F,
	-1.0F,-1.0F,-1.0F,	0.0F, 1.0F, 0.0F,	 0.0F,-1.0F, 0.0F,
	1.0F,-1.0F,-1.0F,	0.0F, 1.0F, 0.0F,	 0.0F,-1.0F, 0.0F,

	// Front Quad
	1.0F,  1.0F, 1.0F,	1.0F, 0.0F, 0.0F,	0.0F, 0.0F, 1.0F,
	-1.0F,  1.0F, 1.0F,	1.0F, 0.0F, 0.0F,	0.0F, 0.0F, 1.0F,
	-1.0F, -1.0F, 1.0F,	1.0F, 0.0F, 0.0F,	0.0F, 0.0F, 1.0F,
	1.0F, -1.0F, 1.0F,	1.0F, 0.0F, 0.0F,	0.0F, 0.0F, 1.0F,

	// Back Quad
	1.0F,-1.0F,-1.0F,	1.0F, 1.0F, 0.0F,	0.0F, 0.0F,-1.0F,
	-1.0F,-1.0F,-1.0F,	1.0F, 1.0F, 0.0F,	0.0F, 0.0F,-1.0F,
	-1.0F, 1.0F,-1.0F,	1.0F, 1.0F, 0.0F,	0.0F, 0.0F,-1.0F,
	1.0F, 1.0F,-1.0F,	1.0F, 1.0F, 0.0F,	0.0F, 0.0F,-1.0F,

	// Left Quad
	-1.0F, 1.0F, 1.0F,	0.0F, 0.0F, 1.0F,	-1.0F, 0.0F, 0.0F,
	-1.0F, 1.0F,-1.0F,	0.0F, 0.0F, 1.0F,	-1.0F, 0.0F, 0.0F,
	-1.0F,-1.0F,-1.0F,	0.0F, 0.0F,	1.0F,	-1.0F, 0.0F, 0.0F,
	-1.0F,-1.0F, 1.0F,	0.0F, 0.0F, 1.0F,	-1.0F, 0.0F, 0.0F,

	// Right Quad
	1.0F, 1.0F,-1.0F,	1.0F, 0.0F, 1.0F,	1.0F, 0.0F, 0.0F,
	1.0F, 1.0F, 1.0F,	1.0F, 0.0F, 1.0F,	1.0F, 0.0F, 0.0F,
	1.0F,-1.0F, 1.0F,	1.0F, 0.0F, 1.0F,	1.0F, 0.0F, 0.0F,
	1.0F,-1.0F,-1.0F,	1.0F, 0.0F, 1.0F,	1.0F, 0.0F, 0.0F
};


static const float TEXTURED_QUBE_VERTEX[192] = {
	/*   position     |    noramal      |    texture    */
	// Top Quad
	1.0F, 1.0F,-1.0F,	 0.0F, 1.0F, 0.0F,	0.34F, 0.0F,
	-1.0F, 1.0F,-1.0F,	 0.0F, 1.0F, 0.0F,	0.0F, 0.0F,
	-1.0F, 1.0F, 1.0F,	 0.0F, 1.0F, 0.0F,	0.0F, 0.338F,
	1.0F, 1.0F, 1.0F,	 0.0F, 1.0F, 0.0F,	0.34F, 0.338F,
	// Bottom Quad
	1.0F,-1.0F, 1.0F,	 0.0F,-1.0F, 0.0F, 	0.33F, 0.338F,
	-1.0F,-1.0F, 1.0F,	 0.0F,-1.0F, 0.0F,	0.669F, 0.338F,
	-1.0F,-1.0F,-1.0F,	 0.0F,-1.0F, 0.0F,	0.669F, 0.0F,
	1.0F,-1.0F,-1.0F,	 0.0F,-1.0F, 0.0F,	0.33F, 0.0F,
	// Front Quad
	1.0F, 1.0F, 1.0F,	 0.0F, 0.0F, 1.0F,	1.0F, 0.0F,
	-1.0F, 1.0F, 1.0F,	 0.0F, 0.0F, 1.0F,	0.663F, 0.0F,
	-1.0F,-1.0F, 1.0F,	 0.0F, 0.0F, 1.0F,	0.663F, 0.3384F,
	1.0F,-1.0F, 1.0F,	 0.0F, 0.0F, 1.0F,	1.0F, 0.3384F,
	// Back Quad
	1.0F,-1.0F,-1.0F,	 0.0F, 0.0F,-1.0F,	0.0F, 0.669F,
	-1.0F,-1.0F,-1.0F,	 0.0F, 0.0F,-1.0F,	0.34F, 0.669F,
	-1.0F, 1.0F,-1.0F,	 0.0F, 0.0F,-1.0F,	0.34F, 0.325F,
	1.0F, 1.0F,-1.0F,	 0.0F, 0.0F,-1.0F,	0.0F, 0.325F,
	// LeFt Quad
	-1.0F, 1.0F, 1.0F,	-1.0F, 0.0F, 0.0F,	0.67F, 0.325F,
	-1.0F, 1.0F,-1.0F,	-1.0F, 0.0F, 0.0F,	0.33F, 0.325F,
	-1.0F,-1.0F,-1.0F,	-1.0F, 0.0F, 0.0F,	0.33F, 0.67F,
	-1.0F,-1.0F, 1.0F,	-1.0F, 0.0F, 0.0F,	0.67F, 0.67F,
	// Right Quad
	1.0F, 1.0F,-1.0F,	1.0F, 0.0F, 0.0F,	 1.0F, 0.325F,
	1.0F, 1.0F, 1.0F,	1.0F, 0.0F, 0.0F,	 0.662F, 0.325F,
	1.0F,-1.0F, 1.0F,	1.0F, 0.0F, 0.0F,	 0.662F, 0.67F,
	1.0F,-1.0F,-1.0F,	1.0F, 0.0F, 0.0F,	 1.0F, 0.67F
};



// TRANGLE_FAN imitation
static const uint32_t CUBE_INDEX[36] = {
	0,1,2,     0,2,3,
	4,5,6,     4,6,7,
	8,9,10,    8,10,11,
	12,13,14,  12,14,15,
	16,17,18,  16,18,19,
	20,21,22,  20,22,23
};

static void tangent_vector(const float * face, float *ret)
{
	glm::vec3 pos1(face[0], face[1], face[2]);
	glm::vec3 pos2(face[8], face[9], face[10]);
	glm::vec3 pos3(face[16], face[17], face[18]);

	glm::vec2 uv1(face[6], face[7]);
	glm::vec2 uv2(face[14], face[15]);
	glm::vec2 uv3(face[22], face[23]);

	glm::vec3 edge1 = pos2 - pos1;
	glm::vec3 edge2 = pos3 - pos1;

	glm::vec2 delta_uv1 = uv2 - uv1;
	glm::vec2 delta_uv2 = uv3 - uv1;

	// tangent
	const float f = 1.0F / ( (delta_uv1.x * delta_uv2.y) - (delta_uv2.x * delta_uv1.y) );
	float x = f * ( (delta_uv2.y * edge1.x) - (delta_uv1.y * edge2.x) );
	float y = f * ( (delta_uv2.y * edge1.y) - (delta_uv1.y * edge2.y) );
	float z = f * ( (delta_uv2.y * edge1.z) - (delta_uv1.y * edge2.z) );

	// normalize
	float inv_length = 1.0F / std::sqrt( (x * x) + (y * y) + (z * z) );
	ret[0] = x * inv_length;
	ret[1] = y * inv_length;
	ret[2] = z * inv_length;
}

// calculate tangents to qube surfaces
static io::scoped_arr<float> calc_tangent_vertex()
{

	constexpr const std::size_t src_vtx_stride = 8;
	constexpr const std::size_t dst_vtx_stride = 11;
	constexpr const std::size_t face_vtx_count = 4;
	constexpr const std::size_t face_count = 6;

	constexpr const std::size_t mesh_size = dst_vtx_stride * face_vtx_count * face_count;

	io::scoped_arr<float> ret(mesh_size);
	float *dst = ret.get();
	const float* src = TEXTURED_QUBE_VERTEX;


	constexpr const std::size_t src_face_size = src_vtx_stride * face_vtx_count;
	constexpr const std::size_t dst_face_size = dst_vtx_stride * face_vtx_count;

#ifdef _OPENMP
	#pragma omp parallel for simd
#endif // _OPENMP
	for (std::size_t i = 0; i < face_count; i++) {
		// index of first face
		std::size_t face_start_idx = i * src_face_size;
		float tan[3];
		tangent_vector( (src + face_start_idx), tan );
		// add tangent vector to each vertex
		for(std::size_t j=0; j < face_vtx_count; j++) {
			std::size_t didx = (i*dst_face_size) + (j*dst_vtx_stride);
			std::size_t sidx = (i*src_face_size) + (j*src_vtx_stride);
			std::memcpy( (dst + didx), (src + sidx), sizeof(float) * src_vtx_stride);
			std::memcpy( (dst + didx + src_vtx_stride), tan, sizeof(tan) );
		}
	}
	return ret;
}


static engine::s_surface textured_qube()
{
	engine::s_image texture_img = engine::load_png_rgba(io::file("cube_tex2d_512x512.png"));
	return engine::s_surface( new engine::textured_mesh(
								  TEXTURED_QUBE_VERTEX, 192,
								  CUBE_INDEX,36, texture_img ) );
}


static engine::s_surface normal_mapped_qube()
{
	engine::s_image diff_tex = engine::load_png_rgb( io::file("face512x512.png") );
	engine::s_image nm_tex = engine::load_png_rgb( io::file("nm512x512.png") );
	io::scoped_arr<float> vertex = calc_tangent_vertex();
	return engine::s_surface( new engine::normal_mapped_mesh(vertex.get(), vertex.len(), CUBE_INDEX,36, diff_tex, nm_tex ) );
}


collada::float_array source_data(const collada::s_source src) {
	collada::s_accessor acsr = * ( src->cbegin() );
	return src->find_float_array( acsr->source_id() );
}


static engine::s_surface load_mesh(const collada::s_model& src_mdl, const collada::mesh* mesh)
{
	using namespace collada;
	collada::float_array pos, nrm, uv;
	// loop over the input and look-up sources
	for(auto it = mesh->cbegin(); it != mesh->cend(); ++it) {
		io::const_string acr_id = it->accessor_id;
		switch(it->type) {
		case semantic_type::position:
			pos = source_data( mesh->find_souce(acr_id) );
			break;
		case semantic_type::normal:
			nrm = source_data( mesh->find_souce(acr_id) );
			break;
		case semantic_type::texcoord:
			uv = source_data( mesh->find_souce(acr_id) );
			break;
		default:
			break;
		}
	}

	unsigned_int_array idx = mesh->index()->indices();
	std::size_t vertex_count = idx.length() / 3;
	std::vector<float> mesh_vertex;
	mesh_vertex.reserve( vertex_count << 3 );
//
	std::cout << "VBO: {\n";
	for(std::size_t i=0; i < idx.length(); i+= 3) {
		std::size_t pos_idx = idx[ i ];
		std::size_t nrm_idx = idx[ i+1 ];
		std::size_t uv_idx = idx[ i+2 ];
		//
		std::cout << pos[ pos_idx   ] << ',';
		std::cout << pos[ pos_idx+1 ] << ',';
		std::cout << pos[ pos_idx+2 ] << ", ";
		mesh_vertex.push_back( pos[ pos_idx ] );
		mesh_vertex.push_back( pos[ pos_idx+1 ] );
		mesh_vertex.push_back( pos[ pos_idx+2 ] );
		//
		std::cout << nrm[ nrm_idx   ] << ',';
		std::cout << nrm[ nrm_idx+1 ] << ',';
		std::cout << nrm[ nrm_idx+2 ] << ", ";

		mesh_vertex.push_back( nrm[ nrm_idx   ] );
		mesh_vertex.push_back( nrm[ nrm_idx+1 ] );
		mesh_vertex.push_back( nrm[ nrm_idx+2 ] );

		//
		std::cout << uv[ uv_idx  ] << ',';
		std::cout << uv[ uv_idx+1] << ',' << std::endl;

		mesh_vertex.push_back( uv[ uv_idx   ] );
		mesh_vertex.push_back( uv[ uv_idx+1 ] );
	}
	std::cout << '}' << std::endl;

	std::vector<unsigned int> mesh_index;
	mesh_index.reserve(vertex_count);
	for(unsigned int i=0; i < vertex_count; i++) {
		mesh_index.push_back( i );
	}

	// TODO: extract from material
	engine::s_image texture_img = engine::load_png_rgba(
										io::file("cube_tex2d_512x512.png")
									);

	const float *vbo = &mesh_vertex.front();
	std::size_t vbo_size = mesh_vertex.size();

	const unsigned int* vio = &mesh_index.front();
	std::size_t vio_size = mesh_index.size();

	std::cout << "vbo size: " << vbo_size << " vio size: " << vio_size << std::endl;

	return  engine::s_surface(
				new engine::textured_mesh(
					vbo,
					vbo_size,
					vio,
					vio_size,
					texture_img
				)
			);
}




static void load_collada_model(engine::s_model& dst_mdl,const char* file_path)
{

	// parse COLLADA file
	io::file dae(file_path);
	std::error_code ec;
	io::s_read_channel src = dae.open_for_read(ec);
	io::check_error_code(ec);
	collada::parser prsr( std::move(src) );
	collada::s_model src_mdl = prsr.load();

	// load parsed COLLADA model into renderer
	collada::s_scene src_scene = src_mdl->scene();
	for( collada::scene::const_iterator it = src_scene->cbegin();
			it != src_scene->cend(); ++it ) {
		if( ! it->geo_ref.url.blank() ) {
			collada::s_geometry geo = src_mdl->find_geometry( it->geo_ref.url );
			std::cout<< "Geometry node name: " << geo->name() << std::endl;
			// TODO: dedicated function for each geometry type
			collada::mesh* src_mesh = reinterpret_cast<collada::mesh*>( geo.get() );
			dst_mdl->add_surface( load_mesh(src_mdl, src_mesh ) );
		}
	}

}

#ifdef _WIN32
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#else
int main(int argc, const char** argv)
#endif // _WIN32
{

	if ( GLFW_TRUE == ::glfwInit() ) {
		try {
			engine::frame_view view(640,480,"Collada model view");

			//engine::s_surface qube( new engine::geometry_mesh(COLORED_QUBE_VERTEX,216,CUBE_INDEX,36) );

			//engine::s_surface qube = textured_qube();

			//engine::s_surface qube = normal_mapped_qube();


			engine::s_model mdl( new engine::model() );
			load_collada_model(mdl, "tex_cube.dae");

			//mdl->add_surface( std::move(qube) );
			view.show( mdl );

			return 0;
		}
		catch(std::exception& exc) {
			std::cerr<< exc.what();
			std::cerr.flush();
		}
		::glfwTerminate();
	}
	return -1;
}

