#ifndef main_H
#define main_H

#include <GL/glew.h>
#pragma comment(lib, "glew32")

#include "uv_camera.h"
#include "custom_math.h"
#include "ogt_vox.h"

#include "shader_utils.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <cstdlib>
#include <GL/glew.h>        // GLEW before GLUT
#include <GL/glut.h>        // GLUT Library

#include <iostream>
using std::cout;
using std::endl;

#include <iomanip>
using std::setprecision;

#include <vector>
using std::vector;

#include <string>
using std::string;

#include <sstream>
using std::ostringstream;
using std::istringstream;

#include <fstream>
using std::ofstream;
using std::ifstream;

#include <set>
using std::set;

#include <map>
using std::map;

#include <utility>
using std::pair;

#include <ios>
using namespace std;

// OpenGL 4 additions
struct Vertex {
	float position[3];
	float color[3];
};

// Function prototypes
void idle_func(void);
void init_opengl(const int& width, const int& height);
void reshape_func(int width, int height);
void display_func(void);
void keyboard_func(unsigned char key, int x, int y);
void mouse_func(int button, int state, int x, int y);
void motion_func(int x, int y);
void passive_motion_func(int x, int y);

void render_string(int x, const int y, void* font, const string& text);
void draw_objects(void);
void cleanup(void);


// Global variables
custom_math::vector_3 background_colour(0.0f, 0.0f, 0.0f);
custom_math::vector_3 control_list_colour(1.0f, 1.0f, 1.0f);

bool draw_axis = true;
bool draw_control_list = true;
bool draw_triangles_on_screen = true;
uv_camera main_camera;

GLint win_id = 0;
GLint win_x = 800, win_y = 600;
float camera_w = 10;

float camera_fov = 45;
float camera_x_transform = 0;
float camera_y_transform = 0;
float u_spacer = 0.01f;
float v_spacer = 0.5f * u_spacer;
float w_spacer = 0.1f;
float camera_near = 0.01f;
float camera_far = 100.0f;

bool lmb_down = false;
bool mmb_down = false;
bool rmb_down = false;
int mouse_x = 0;
int mouse_y = 0;

// Triangles data
vector<custom_math::triangle> tri_vec;
custom_math::vertex_3 min_location, max_location;

vector<custom_math::vertex_3> voxel_centres;
vector<float> voxel_densities;

vector<custom_math::vertex_3> background_grid_points;







// Add this to your header file
struct VoxelGrid {
	// Grid parameters
	float voxel_size;
	custom_math::vertex_3 grid_min;
	custom_math::vertex_3 grid_max;
	int grid_size_x, grid_size_y, grid_size_z;

	// 3D array of voxel indices (stores -1 for empty cells)
	std::vector<int> grid_cells;

	// Initialize the grid based on voxel data
	void initialize(const std::vector<custom_math::vertex_3>& voxel_centres,
		const std::vector<float>& voxel_densities,
		float cell_size = 1.0f) {
		voxel_size = cell_size;

		// Find min/max extents
		if (voxel_centres.empty()) return;

		grid_min = voxel_centres[0];
		grid_max = voxel_centres[0];

		for (const auto& center : voxel_centres) {
			grid_min.x = std::min(grid_min.x, center.x - voxel_size / 2.0f);
			grid_min.y = std::min(grid_min.y, center.y - voxel_size / 2.0f);
			grid_min.z = std::min(grid_min.z, center.z - voxel_size / 2.0f);

			grid_max.x = std::max(grid_max.x, center.x + voxel_size / 2.0f);
			grid_max.y = std::max(grid_max.y, center.y + voxel_size / 2.0f);
			grid_max.z = std::max(grid_max.z, center.z + voxel_size / 2.0f);
		}

		// Add padding
		const float padding = voxel_size * 0.1f;
		grid_min.x -= padding;
		grid_min.y -= padding;
		grid_min.z -= padding;
		grid_max.x += padding;
		grid_max.y += padding;
		grid_max.z += padding;

		// Calculate grid dimensions
		float size_x = grid_max.x - grid_min.x;
		float size_y = grid_max.y - grid_min.y;
		float size_z = grid_max.z - grid_min.z;

		grid_size_x = static_cast<int>(std::ceil(size_x / voxel_size));
		grid_size_y = static_cast<int>(std::ceil(size_y / voxel_size));
		grid_size_z = static_cast<int>(std::ceil(size_z / voxel_size));

		// Initialize grid with -1 (empty)
		grid_cells.resize(grid_size_x * grid_size_y * grid_size_z, -1);

		// Place voxels in the grid
		for (size_t i = 0; i < voxel_centres.size(); i++) {
			if (voxel_densities[i] <= 0.0f) continue;

			const auto& center = voxel_centres[i];

			// Get grid cell coordinates
			int cell_x = static_cast<int>((center.x - grid_min.x) / voxel_size);
			int cell_y = static_cast<int>((center.y - grid_min.y) / voxel_size);
			int cell_z = static_cast<int>((center.z - grid_min.z) / voxel_size);

			// Ensure within bounds
			cell_x = std::max(0, std::min(cell_x, grid_size_x - 1));
			cell_y = std::max(0, std::min(cell_y, grid_size_y - 1));
			cell_z = std::max(0, std::min(cell_z, grid_size_z - 1));

			// Get index in the flattened 3D array
			size_t cell_index = cell_x + (cell_y * grid_size_x) + (cell_z * grid_size_x * grid_size_y);

			// Store voxel index in the grid
			if (cell_index < grid_cells.size()) {
				grid_cells[cell_index] = static_cast<int>(i);
			}
		}
	}

	// Find which voxel contains a point
	bool find_voxel_containing_point(const custom_math::vertex_3& point,
		size_t& voxel_index) const {
		// Get grid cell coordinates
		int cell_x = static_cast<int>((point.x - grid_min.x) / voxel_size);
		int cell_y = static_cast<int>((point.y - grid_min.y) / voxel_size);
		int cell_z = static_cast<int>((point.z - grid_min.z) / voxel_size);

		// Check bounds
		if (cell_x < 0 || cell_x >= grid_size_x ||
			cell_y < 0 || cell_y >= grid_size_y ||
			cell_z < 0 || cell_z >= grid_size_z) {
			return false;  // Outside grid
		}

		// Find the index in the flattened 3D array
		size_t cell_index = cell_x + (cell_y * grid_size_x) + (cell_z * grid_size_x * grid_size_y);

		int voxel_idx = grid_cells[cell_index];

		if (voxel_idx == -1) {
			return false;  // No voxel here
		}

		// Do a precise check against the voxel
		const float half_size = voxel_size * 0.5f;
		const custom_math::vertex_3& center = voxel_centres[voxel_idx];

		if (point.x >= center.x - half_size &&
			point.x <= center.x + half_size &&
			point.y >= center.y - half_size &&
			point.y <= center.y + half_size &&
			point.z >= center.z - half_size &&
			point.z <= center.z + half_size) {

			voxel_index = voxel_idx;
			return true;
		}

		return false;
	}
};

// Combine the grid with the model transformation
bool is_point_in_voxel_grid(const custom_math::vertex_3& test_point,
	const glm::mat4& model,
	const VoxelGrid& grid,
	size_t& voxel_index) {
	// 1. Calculate the inverse model matrix
	glm::mat4 inv_model_matrix = glm::inverse(model);

	// 2. Transform the test point with the inverse model matrix
	glm::vec4 model_space_point(test_point.x, test_point.y, test_point.z, 1.0f);
	glm::vec4 local_space_point = inv_model_matrix * model_space_point;

	// 3. Create a vertex_3 from the transformed point
	custom_math::vertex_3 transformed_point(
		local_space_point.x,
		local_space_point.y,
		local_space_point.z
	);

	// 4. Use the grid to find the voxel
	return grid.find_voxel_containing_point(transformed_point, voxel_index);
}




VoxelGrid voxel_grid;



		
glm::mat4 model_matrix = glm::mat4(1.0f);
float u = 0.0f, v = 0.0f;


void calc_AABB_min_max_locations(void)
{
	float x_min = numeric_limits<float>::max();
	float y_min = numeric_limits<float>::max();
	float z_min = numeric_limits<float>::max();
	float x_max = -numeric_limits<float>::max();
	float y_max = -numeric_limits<float>::max();
	float z_max = -numeric_limits<float>::max();

	for (size_t t = 0; t < tri_vec.size(); t++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			if (tri_vec[t].vertex[j].x < x_min)
				x_min = tri_vec[t].vertex[j].x;

			if (tri_vec[t].vertex[j].x > x_max)
				x_max = tri_vec[t].vertex[j].x;

			if (tri_vec[t].vertex[j].y < y_min)
				y_min = tri_vec[t].vertex[j].y;

			if (tri_vec[t].vertex[j].y > y_max)
				y_max = tri_vec[t].vertex[j].y;

			if (tri_vec[t].vertex[j].z < z_min)
				z_min = tri_vec[t].vertex[j].z;

			if (tri_vec[t].vertex[j].z > z_max)
				z_max = tri_vec[t].vertex[j].z;
		}
	}

	min_location.x = x_min;
	min_location.y = y_min;
	min_location.z = z_min;

	max_location.x = x_max;
	max_location.y = y_max;
	max_location.z = z_max;
}

void centre_mesh_on_xyz(void)
{
	float x_min = numeric_limits<float>::max();
	float y_min = numeric_limits<float>::max();
	float z_min = numeric_limits<float>::max();
	float x_max = -numeric_limits<float>::max();
	float y_max = -numeric_limits<float>::max();
	float z_max = -numeric_limits<float>::max();

	for (size_t t = 0; t < tri_vec.size(); t++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			if (tri_vec[t].vertex[j].x < x_min)
				x_min = tri_vec[t].vertex[j].x;

			if (tri_vec[t].vertex[j].x > x_max)
				x_max = tri_vec[t].vertex[j].x;

			if (tri_vec[t].vertex[j].y < y_min)
				y_min = tri_vec[t].vertex[j].y;

			if (tri_vec[t].vertex[j].y > y_max)
				y_max = tri_vec[t].vertex[j].y;

			if (tri_vec[t].vertex[j].z < z_min)
				z_min = tri_vec[t].vertex[j].z;

			if (tri_vec[t].vertex[j].z > z_max)
				z_max = tri_vec[t].vertex[j].z;
		}
	}

	for (size_t t = 0; t < tri_vec.size(); t++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			tri_vec[t].vertex[j].x += -(x_max + x_min) / 2.0f;
			tri_vec[t].vertex[j].y += -(y_max + y_min) / 2.0f;
			tri_vec[t].vertex[j].z += -(z_max + z_min) / 2.0f;
		}
	}

	for (size_t t = 0; t < voxel_centres.size(); t++)
	{

			voxel_centres[t].x += -(x_max + x_min) / 2.0f;
			voxel_centres[t].y += -(y_max + y_min) / 2.0f;
			voxel_centres[t].z += -(z_max + z_min) / 2.0f;
	}

}

bool write_triangles_to_binary_stereo_lithography_file(const vector<custom_math::triangle>& triangles, const char* const file_name)
{
	cout << "Triangle count: " << triangles.size() << endl;

	if (0 == triangles.size())
		return false;

	// Write to file.
	ofstream out(file_name, ios_base::binary);

	if (out.fail())
		return false;

	const size_t header_size = 80;
	vector<char> buffer(header_size, 0);
	const unsigned int num_triangles = static_cast<unsigned int>(triangles.size()); // Must be 4-byte unsigned int.
	custom_math::vertex_3 normal;

	// Write blank header.
	out.write(reinterpret_cast<const char*>(&(buffer[0])), header_size);

	// Write number of triangles.
	out.write(reinterpret_cast<const char*>(&num_triangles), sizeof(unsigned int));

	// Copy everything to a single buffer.
	cout << "Generating normal/vertex/attribute buffer" << endl;

	// Enough bytes for twelve 4-byte floats plus one 2-byte integer, per triangle.
	const size_t data_size = (12 * sizeof(float) + sizeof(short unsigned int)) * num_triangles;
	buffer.resize(data_size, 0);

	// Use a pointer to assist with the copying.
	char* cp = &buffer[0];

	for (vector<custom_math::triangle>::const_iterator i = triangles.begin(); i != triangles.end(); i++)
	{
		// Get face normal.
		custom_math::vertex_3 v0 = i->vertex[1] - i->vertex[0];
		custom_math::vertex_3 v1 = i->vertex[2] - i->vertex[0];
		normal = v0.cross(v1);
		normal.normalize();

		memcpy(cp, &normal.x, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &normal.y, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &normal.z, sizeof(float)); cp += sizeof(float);

		memcpy(cp, &i->vertex[0].x, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &i->vertex[0].y, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &i->vertex[0].z, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &i->vertex[1].x, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &i->vertex[1].y, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &i->vertex[1].z, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &i->vertex[2].x, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &i->vertex[2].y, sizeof(float)); cp += sizeof(float);
		memcpy(cp, &i->vertex[2].z, sizeof(float)); cp += sizeof(float);

		cp += sizeof(short unsigned int);
	}

	cout << "Writing " << data_size / 1048576.0f << " MB of data to binary Stereo Lithography file: " << file_name << endl;

	out.write(reinterpret_cast<const char*>(&buffer[0]), data_size);
	out.close();

	return true;
}

bool read_quads_from_vox_file(string file_name, vector<custom_math::triangle>& tri_vec)
{
	tri_vec.clear();
	voxel_centres.clear();
	voxel_densities.clear();

	ifstream infile(file_name, ifstream::ate | ifstream::binary);

	if (infile.fail())
	{
		cout << "Could not open file " << file_name << endl;
		return false;
	}

	size_t file_size = infile.tellg();

	infile.close();

	if (file_size == 0)
		return false;

	infile.open(file_name, ifstream::binary);

	if (infile.fail())
	{
		return false;
	}

	vector<unsigned char> v(file_size, 0);

	infile.read(reinterpret_cast<char*>(&v[0]), file_size);
	infile.close();

	const ogt_vox_scene* scene = ogt_vox_read_scene(&v[0], static_cast<uint32_t>(file_size));

	voxel_centres.resize(scene->models[0]->size_x * scene->models[0]->size_y * scene->models[0]->size_z);
	voxel_densities.resize(scene->models[0]->size_x * scene->models[0]->size_y * scene->models[0]->size_z);

	cout << voxel_centres.size() << endl;

	for (size_t x = 0; x < scene->models[0]->size_x; x++)
	{
		for (size_t y = 0; y < scene->models[0]->size_y; y++)
		{
			for (size_t z = 0; z < scene->models[0]->size_z; z++)
			{
				float scale = 1.0;

				const size_t voxel_index = x + (y * scene->models[0]->size_x) + (z * scene->models[0]->size_x * scene->models[0]->size_y);
				const uint8_t colour_index = scene->models[0]->voxel_data[voxel_index];

				custom_math::vertex_3 translate(x * scale, y * scale, z * scale);

				voxel_centres[voxel_index] = translate;



				// Transparent
				if (colour_index == 0)
				{
					voxel_densities[voxel_index] = 0.0;
					continue;
				}
				else
				{
					voxel_densities[voxel_index] = 1.0;
				}

				const ogt_vox_rgba colour = scene->palette.color[colour_index];

				// Now you can use the individual components
				uint8_t r = colour.r;
				uint8_t g = colour.g;
				uint8_t b = colour.b;
				uint8_t a = colour.a;  // Alpha channel


				custom_math::quad q0, q1, q2, q3, q4, q5;

				size_t neighbour_index = 0;

				// Top face (y = 1.0f)
				q0.vertex[0] = custom_math::vertex_3(scale * 0.5f, scale * 0.5f, -scale * 0.5f) + translate;
				q0.vertex[1] = custom_math::vertex_3(-scale * 0.5f, scale * 0.5f, -scale * 0.5f) + translate;
				q0.vertex[2] = custom_math::vertex_3(-scale * 0.5f, scale * 0.5f, scale * 0.5f) + translate;
				q0.vertex[3] = custom_math::vertex_3(scale * 0.5f, scale * 0.5f, scale * 0.5f) + translate;

				q0.colour.x = r / 255.0f;
				q0.colour.y = g / 255.0f;
				q0.colour.z = b / 255.0f;

				// Bottom face (y = -scale*0.5f)
				q1.vertex[0] = custom_math::vertex_3(scale * 0.5f, -scale * 0.5f, scale * 0.5f) + translate;
				q1.vertex[1] = custom_math::vertex_3(-scale * 0.5f, -scale * 0.5f, scale * 0.5f) + translate;
				q1.vertex[2] = custom_math::vertex_3(-scale * 0.5f, -scale * 0.5f, -scale * 0.5f) + translate;
				q1.vertex[3] = custom_math::vertex_3(scale * 0.5f, -scale * 0.5f, -scale * 0.5f) + translate;

				q1.colour.x = r / 255.0f;
				q1.colour.y = g / 255.0f;
				q1.colour.z = b / 255.0f;

				// Front face  (z = scale*0.5f)
				q2.vertex[0] = custom_math::vertex_3(scale * 0.5f, scale * 0.5f, scale * 0.5f) + translate;
				q2.vertex[1] = custom_math::vertex_3(-scale * 0.5f, scale * 0.5f, scale * 0.5f) + translate;
				q2.vertex[2] = custom_math::vertex_3(-scale * 0.5f, -scale * 0.5f, scale * 0.5f) + translate;
				q2.vertex[3] = custom_math::vertex_3(scale * 0.5f, -scale * 0.5f, scale * 0.5f) + translate;

				q2.colour.x = r / 255.0f;
				q2.colour.y = g / 255.0f;
				q2.colour.z = b / 255.0f;

				// Back face (z = -scale*0.5f)
				q3.vertex[0] = custom_math::vertex_3(scale * 0.5f, -scale * 0.5f, -scale * 0.5f) + translate;
				q3.vertex[1] = custom_math::vertex_3(-scale * 0.5f, -scale * 0.5f, -scale * 0.5f) + translate;
				q3.vertex[2] = custom_math::vertex_3(-scale * 0.5f, scale * 0.5f, -scale * 0.5f) + translate;
				q3.vertex[3] = custom_math::vertex_3(scale * 0.5f, scale * 0.5f, -scale * 0.5f) + translate;

				q3.colour.x = r / 255.0f;
				q3.colour.y = g / 255.0f;
				q3.colour.z = b / 255.0f;

				// Right face (x = scale*0.5f)
				q4.vertex[0] = custom_math::vertex_3(scale * 0.5f, scale * 0.5f, -scale * 0.5f) + translate;
				q4.vertex[1] = custom_math::vertex_3(scale * 0.5f, scale * 0.5f, scale * 0.5f) + translate;
				q4.vertex[2] = custom_math::vertex_3(scale * 0.5f, -scale * 0.5f, scale * 0.5f) + translate;
				q4.vertex[3] = custom_math::vertex_3(scale * 0.5f, -scale * 0.5f, -scale * 0.5f) + translate;
				q4.vertex[0].nx = 1;
				q4.vertex[1].nx = 1;
				q4.vertex[2].nx = 1;
				q4.vertex[3].nx = 1;

				q4.colour.x = r / 255.0f;
				q4.colour.y = g / 255.0f;
				q4.colour.z = b / 255.0f;

				// Left face (x = -scale*0.5f)
				q5.vertex[0] = custom_math::vertex_3(-scale * 0.5f, scale * 0.5f, scale * 0.5f) + translate;
				q5.vertex[1] = custom_math::vertex_3(-scale * 0.5f, scale * 0.5f, -scale * 0.5f) + translate;
				q5.vertex[2] = custom_math::vertex_3(-scale * 0.5f, -scale * 0.5f, -scale * 0.5f) + translate;
				q5.vertex[3] = custom_math::vertex_3(-scale * 0.5f, -scale * 0.5f, scale * 0.5f) + translate;
				q5.vertex[0].nx = -1;
				q5.vertex[1].nx = -1;
				q5.vertex[2].nx = -1;
				q5.vertex[3].nx = -1;

				q5.colour.x = r / 255.0f;
				q5.colour.y = g / 255.0f;
				q5.colour.z = b / 255.0f;

				custom_math::triangle t;

				t.colour.x = r / 255.0f;
				t.colour.y = g / 255.0f;
				t.colour.z = b / 255.0f;

				// Note that this index is possibly out of range, 
				// which is why it's used second in the if()
				neighbour_index = x + (y + 1) * scene->models[0]->size_x + z * scene->models[0]->size_x * scene->models[0]->size_y;
				if (y == scene->models[0]->size_y - 1 || 0 == scene->models[0]->voxel_data[neighbour_index])
				{
					t.vertex[0] = q0.vertex[0];
					t.vertex[1] = q0.vertex[1];
					t.vertex[2] = q0.vertex[2];

					tri_vec.push_back(t);

					t.vertex[0] = q0.vertex[0];
					t.vertex[1] = q0.vertex[2];
					t.vertex[2] = q0.vertex[3];
					tri_vec.push_back(t);
				}

				// Note that this index is possibly out of range, 
				// which is why it's used second in the if()
				neighbour_index = x + (y - 1) * scene->models[0]->size_x + z * scene->models[0]->size_x * scene->models[0]->size_y;
				if (y == 0 || 0 == scene->models[0]->voxel_data[neighbour_index])
				{
					t.vertex[0] = q1.vertex[0];
					t.vertex[1] = q1.vertex[1];
					t.vertex[2] = q1.vertex[2];
					tri_vec.push_back(t);

					t.vertex[0] = q1.vertex[0];
					t.vertex[1] = q1.vertex[2];
					t.vertex[2] = q1.vertex[3];
					tri_vec.push_back(t);
				}


				// Note that this index is possibly out of range, 
				// which is why it's used second in the if()
				neighbour_index = x + y * scene->models[0]->size_x + (z + 1) * scene->models[0]->size_x * scene->models[0]->size_y;
				if (z == scene->models[0]->size_z - 1 || 0 == scene->models[0]->voxel_data[neighbour_index])
				{
					t.vertex[0] = q2.vertex[0];
					t.vertex[1] = q2.vertex[1];
					t.vertex[2] = q2.vertex[2];
					tri_vec.push_back(t);

					t.vertex[0] = q2.vertex[0];
					t.vertex[1] = q2.vertex[2];
					t.vertex[2] = q2.vertex[3];
					tri_vec.push_back(t);
				}


				// Note that this index is possibly out of range, 
				// which is why it's used second in the if()
				neighbour_index = x + (y)*scene->models[0]->size_x + (z - 1) * scene->models[0]->size_x * scene->models[0]->size_y;
				if (z == 0 || 0 == scene->models[0]->voxel_data[neighbour_index])
				{
					t.vertex[0] = q3.vertex[0];
					t.vertex[1] = q3.vertex[1];
					t.vertex[2] = q3.vertex[2];
					tri_vec.push_back(t);

					t.vertex[0] = q3.vertex[0];
					t.vertex[1] = q3.vertex[2];
					t.vertex[2] = q3.vertex[3];
					tri_vec.push_back(t);
				}


				// Note that this index is possibly out of range, 
				// which is why it's used second in the if()
				neighbour_index = (x + 1) + (y)*scene->models[0]->size_x + (z)*scene->models[0]->size_x * scene->models[0]->size_y;
				if (x == scene->models[0]->size_x - 1 || 0 == scene->models[0]->voxel_data[neighbour_index])
				{
					t.vertex[0] = q4.vertex[0];
					t.vertex[1] = q4.vertex[1];
					t.vertex[2] = q4.vertex[2];
					tri_vec.push_back(t);

					t.vertex[0] = q4.vertex[0];
					t.vertex[1] = q4.vertex[2];
					t.vertex[2] = q4.vertex[3];
					tri_vec.push_back(t);
				}

				// Note that this index is possibly out of range, 
				// which is why it's used second in the if()
				neighbour_index = (x - 1) + (y)*scene->models[0]->size_x + (z)*scene->models[0]->size_x * scene->models[0]->size_y;
				if (x == 0 || 0 == scene->models[0]->voxel_data[neighbour_index])
				{
					t.vertex[0] = q5.vertex[0];
					t.vertex[1] = q5.vertex[1];
					t.vertex[2] = q5.vertex[2];
					tri_vec.push_back(t);

					t.vertex[0] = q5.vertex[0];
					t.vertex[1] = q5.vertex[2];
					t.vertex[2] = q5.vertex[3];
					tri_vec.push_back(t);
				}
			}
		}
	}

	ogt_vox_destroy_scene(scene);



	for (size_t i = 0; i < tri_vec.size(); i++)
	{
		static const float pi = 4.0f * atanf(1.0f);

		tri_vec[i].vertex[0].rotate_x(pi - pi / 2.0f);
		tri_vec[i].vertex[1].rotate_x(pi - pi / 2.0f);
		tri_vec[i].vertex[2].rotate_x(pi - pi / 2.0f);
	}

	for (size_t i = 0; i < voxel_centres.size(); i++)
	{
		static const float pi = 4.0f * atanf(1.0f);

		voxel_centres[i].rotate_x(pi - pi / 2.0f);
	}


	centre_mesh_on_xyz();

	calc_AABB_min_max_locations();

	return true;
}



void get_background_points(vector<custom_math::vertex_3>& points)
{
	points.clear();

	size_t res = 30;

	float x_grid_max = 20;
	float y_grid_max = 20;
	float z_grid_max = 20;
	float x_grid_min = -x_grid_max;
	float y_grid_min = -y_grid_max;
	float z_grid_min = -z_grid_max;
	size_t x_res = res;
	size_t y_res = res;
	size_t z_res = res;

	const float x_step_size = (x_grid_max - x_grid_min) / (x_res - 1);
	const float y_step_size = (y_grid_max - y_grid_min) / (y_res - 1);
	const float z_step_size = (z_grid_max - z_grid_min) / (z_res - 1);

	custom_math::vertex_3 Z(x_grid_min, y_grid_min, x_grid_min);

	for (size_t z = 0; z < z_res; z++, Z.z += z_step_size)
	{
		Z.x = x_grid_min;

		for (size_t x = 0; x < x_res; x++, Z.x += x_step_size)
		{
			Z.y = y_grid_min;

			for (size_t y = 0; y < y_res; y++, Z.y += y_step_size)
			{
				custom_math::vertex_3 test_point(Z.x, Z.y, Z.z);

				size_t voxel_index = 0;

				if (is_point_in_voxel_grid(test_point, model_matrix, voxel_grid, voxel_index)) 
				{
					points.push_back(test_point);
				}
			}
		}
	}
}






#endif
