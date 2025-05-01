#ifndef main_H
#define main_H

#include "uv_camera.h"
#include "custom_math.h"
#include "ogt_vox.h"
#include "ogt_voxel_meshify.h"


#include <cstdlib>
#include <GL/glut.h>       //GLUT Library

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



void idle_func(void);
void init_opengl(const int &width, const int &height);
void reshape_func(int width, int height);
void display_func(void);
void keyboard_func(unsigned char key, int x, int y);
void mouse_func(int button, int state, int x, int y);
void motion_func(int x, int y);
void passive_motion_func(int x, int y);

void render_string(int x, const int y, void *font, const string &text);
void draw_objects(void);


custom_math::vector_3 background_colour(0.0f, 0.0f, 0.0f);
custom_math::vector_3 control_list_colour(1.0f, 1.0f, 1.0f);

bool draw_axis = true;
bool draw_control_list = true;

uv_camera main_camera;

GLint win_id = 0;
GLint win_x = 800, win_y = 600;
float camera_w = 10;

float camera_fov = 45;
float camera_x_transform = 0;
float camera_y_transform = 0;
float u_spacer = 0.01f;
float v_spacer = 0.5f*u_spacer;
float w_spacer = 0.1f;
float camera_near = 0.01;
float camera_far = 100.0;

bool lmb_down = false;
bool mmb_down = false;
bool rmb_down = false;
int mouse_x = 0;
int mouse_y = 0;



vector<custom_math::triangle> tri_vec;



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
	// We do this here because calling ofstream::write() only once PER MESH is going to 
	// send the data to disk faster than if we were to instead call ofstream::write()
	// thirteen times PER TRIANGLE.
	// Of course, the trade-off is that we are using 2x the RAM than what's absolutely required,
	// but the trade-off is often very much worth it (especially so for meshes with millions of triangles).
	cout << "Generating normal/vertex/attribute buffer" << endl;

	// Enough bytes for twelve 4-byte floats plus one 2-byte integer, per triangle.
	const size_t data_size = (12 * sizeof(float) + sizeof(short unsigned int)) * num_triangles;
	buffer.resize(data_size, 0);

	// Use a pointer to assist with the copying.
	// Should probably use std::copy() instead, but memcpy() does the trick, so whatever...
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
		//cout << "Could not re-open file " << file_name << endl;
		return false;
	}

	vector<unsigned char> v(file_size, 0);

	infile.read(reinterpret_cast<char*>(&v[0]), file_size);
	infile.close();

	const ogt_vox_scene* scene = ogt_vox_read_scene(&v[0], file_size);

	//ogt_mesh* mesh = ogt_mesh_from_paletted_voxels_simple(scene->models[0]->voxel_data, size_x, size_y, size_z, voxel_palette);


	for (size_t x = 0; x < scene->models[0]->size_x; x++)
	{
		for (size_t y = 0; y < scene->models[0]->size_y; y++)
		{
			for (size_t z = 0; z < scene->models[0]->size_z; z++)
			{
				float scale = 1.0;

				const size_t voxel_index = x + (y * scene->models[0]->size_x) + (z * scene->models[0]->size_x * scene->models[0]->size_y);
				const uint8_t colour_index = scene->models[0]->voxel_data[voxel_index];


				// Transparent
				if (colour_index == 0)
					continue;



				const ogt_vox_rgba colour = scene->palette.color[colour_index];

				// Now you can use the individual components
				uint8_t r = colour.r;
				uint8_t g = colour.g;
				uint8_t b = colour.b;
				uint8_t a = colour.a;  // Alpha channel

				custom_math::vertex_3 translate(x * scale, y * scale, z * scale);

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

				neighbour_index = x + (y - 1) * scene->models[0]->size_x + z * scene->models[0]->size_x * scene->models[0]->size_y;
				if (y == 0 || 0 == scene->models[0]->voxel_data[neighbour_index])
				{
					if (1)//cull_faces == false) // || q1.vertex[3].nx < 0 || q1.vertex[3].nz > 0 || q1.vertex[3].ny == 1)
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
				}

				neighbour_index = x + y*scene->models[0]->size_x + (z + 1) * scene->models[0]->size_x * scene->models[0]->size_y;
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


				neighbour_index = x + (y)*scene->models[0]->size_x + (z - 1) * scene->models[0]->size_x * scene->models[0]->size_y;
				if (z == 0 || 0 == scene->models[0]->voxel_data[neighbour_index])
				{
					if (1)//cull_faces == false) //|| q3.vertex[3].nx < 0 || q3.vertex[3].nz > 0 || q3.vertex[3].ny == 1)
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
				}

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

				neighbour_index = (x - 1) + (y)*scene->models[0]->size_x + (z)*scene->models[0]->size_x * scene->models[0]->size_y;
				if (x == 0 || 0 == scene->models[0]->voxel_data[neighbour_index])
				{
					if (1)//cull_faces == false)// || q5.vertex[3].nx < 0 || q5.vertex[3].nz > 0 || q5.vertex[3].ny == 1)
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
	}

	ogt_vox_destroy_scene(scene);

	centre_mesh_on_xyz();



	return true;
}








#endif
