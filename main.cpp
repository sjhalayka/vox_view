#include "main.h"
#include "shader_utils.h"




// Vertex structure for shader compatibility
struct RenderVertex {
    float position[3];
    float color[3];
};

// Common vertex shader for all primitives
const char* commonVertexShaderSource = R"(
#version 430 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec3 fragColor;
void main() {
    fragColor = color;
    gl_Position = projection * view * model * vec4(position, 1.0);
}
)";

// Common fragment shader for all primitives
const char* commonFragmentShaderSource = R"(
#version 430 core
in vec3 fragColor;
out vec4 finalColor;
void main() {
    finalColor = vec4(fragColor, 1.0);
}
)";

void draw_triangles(const std::vector<custom_math::vertex_3>& positions, const std::vector<custom_math::vertex_3>& colors, glm::mat4 model) {
    if (positions.empty() || colors.empty() || positions.size() != colors.size()) {
        return;
    }

    // Calculate triangle indices
    std::vector<GLuint> indices;
    for (size_t i = 0; i < positions.size(); i += 3) {
        if (i + 2 < positions.size()) {
            indices.push_back(static_cast<GLuint>(i));
            indices.push_back(static_cast<GLuint>(i + 1));
            indices.push_back(static_cast<GLuint>(i + 2));
        }
    }

    // Create vertex data
    std::vector<RenderVertex> vertices;
    for (size_t i = 0; i < positions.size(); ++i) {
        RenderVertex vertex;
        vertex.position[0] = positions[i].x;
        vertex.position[1] = positions[i].y;
        vertex.position[2] = positions[i].z;
        vertex.color[0] = colors[i].x;
        vertex.color[1] = colors[i].y;
        vertex.color[2] = colors[i].z;
        vertices.push_back(vertex);
    }

    // Create shader program
    GLuint shaderProgram = createShaderProgram(commonVertexShaderSource, commonFragmentShaderSource);
    if (shaderProgram == 0) {
        return;
    }

    // Create and bind VAO
    GLuint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create and bind VBO
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(RenderVertex), vertices.data(), GL_STATIC_DRAW);

    // Create and bind EBO
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    // Set vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Use shader program
    glUseProgram(shaderProgram);

    // Set uniform matrices
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(main_camera.view_mat));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(main_camera.projection_mat));

    // Draw triangles
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

    // Cleanup
    glBindVertexArray(0);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shaderProgram);
}

void draw_lines(const std::vector<custom_math::vertex_3>& positions, const std::vector<custom_math::vertex_3>& colors, glm::mat4 model) {
    if (positions.empty() || colors.empty() || positions.size() != colors.size()) {
        return;
    }

    // Create vertex data
    std::vector<RenderVertex> vertices;
    for (size_t i = 0; i < positions.size(); ++i) {
        RenderVertex vertex;
        vertex.position[0] = positions[i].x;
        vertex.position[1] = positions[i].y;
        vertex.position[2] = positions[i].z;
        vertex.color[0] = colors[i].x;
        vertex.color[1] = colors[i].y;
        vertex.color[2] = colors[i].z;
        vertices.push_back(vertex);
    }

    // Create shader program
    GLuint shaderProgram = createShaderProgram(commonVertexShaderSource, commonFragmentShaderSource);
    if (shaderProgram == 0) {
        return;
    }

    // Create and bind VAO
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create and bind VBO
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(RenderVertex), vertices.data(), GL_STATIC_DRAW);

    // Set vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Use shader program
    glUseProgram(shaderProgram);

    // Set uniform matrices
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    //glm::mat4 model = glm::mat4(1.0f);
    //model = glm::rotate(model, u, glm::vec3(0.0f, 1.0f, 0.0f));
    //model = glm::rotate(model, v, glm::vec3(1.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(main_camera.view_mat));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(main_camera.projection_mat));

    // Draw lines
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertices.size()));

    // Cleanup
    glBindVertexArray(0);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shaderProgram);
}

void draw_points(const std::vector<custom_math::vertex_3>& positions, const std::vector<custom_math::vertex_3>& colors, glm::mat4 model) {
    if (positions.empty() || colors.empty() || positions.size() != colors.size()) {
        return;
    }

    // Create vertex data
    std::vector<RenderVertex> vertices;
    for (size_t i = 0; i < positions.size(); ++i) {
        RenderVertex vertex;
        vertex.position[0] = positions[i].x;
        vertex.position[1] = positions[i].y;
        vertex.position[2] = positions[i].z;
        vertex.color[0] = colors[i].x;
        vertex.color[1] = colors[i].y;
        vertex.color[2] = colors[i].z;
        vertices.push_back(vertex);
    }

    // Create shader program
    GLuint shaderProgram = createShaderProgram(commonVertexShaderSource, commonFragmentShaderSource);
    if (shaderProgram == 0) {
        return;
    }

    // Create and bind VAO
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create and bind VBO
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(RenderVertex), vertices.data(), GL_STATIC_DRAW);

    // Set vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(RenderVertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Use shader program
    glUseProgram(shaderProgram);

    // Set uniform matrices
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(main_camera.view_mat));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(main_camera.projection_mat));

    // Draw points
    glPointSize(5.0f); // Optional: set point size
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(vertices.size()));

    // Cleanup
    glBindVertexArray(0);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shaderProgram);
}


int main(int argc, char** argv)
{
    cout << setprecision(20) << endl;

   // read_scene_from_vox_file("chr_knight.vox");

    get_triangles("chr_knight.vox", tri_vec);
    voxel_grid.initialize(voxel_centres, voxel_densities);
    get_background_points();
    get_surface_points();

  ///  get_surface_points(background_grid_points);


    glutInit(&argc, argv);
    init_opengl(win_x, win_y);

    // Initialize GLEW after GLUT and context creation
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        cerr << "Error: " << glewGetErrorString(err) << endl;
        return 1;
    }

    glutReshapeFunc(reshape_func);
    glutIdleFunc(idle_func);
    glutDisplayFunc(display_func);
    glutKeyboardFunc(keyboard_func);
    glutMouseFunc(mouse_func);
    glutMotionFunc(motion_func);
    glutPassiveMotionFunc(passive_motion_func);
//    glutCloseFunc(cleanup);

    glutMainLoop();

    return 0;
}


void idle_func(void)
{
    glutPostRedisplay();
}

void init_opengl(const int& width, const int& height)
{
    win_x = width;
    win_y = height;

    if (win_x < 1)
        win_x = 1;

    if (win_y < 1)
        win_y = 1;

    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(win_x, win_y);
    win_id = glutCreateWindow("OpenGL 4 Mesh Viewer");

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glClearColor((float)background_colour.x, (float)background_colour.y, (float)background_colour.z, 1);
    glClearDepth(1.0f);

    main_camera.calculate_camera_matrices(win_x, win_y);


}

void reshape_func(int width, int height)
{
    win_x = width;
    win_y = height;

    if (win_x < 1)
        win_x = 1;

    if (win_y < 1)
        win_y = 1;

    glutSetWindow(win_id);
    glutReshapeWindow(win_x, win_y);
    glViewport(0, 0, win_x, win_y);

    main_camera.calculate_camera_matrices(win_x, win_y);
}

// Text drawing code originally from "GLUT Tutorial -- Bitmap Fonts and Orthogonal Projections" by A R Fernandes
void render_string(int x, const int y, void* font, const string& text)
{
    glUseProgram(0); // Disable shaders for GLUT text rendering

    // Setup orthographic projection for text rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, win_x, 0, win_y);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    for (size_t i = 0; i < text.length(); i++)
    {
        glRasterPos2i(x, y);
        glutBitmapCharacter(font, text[i]);
        x += glutBitmapWidth(font, text[i]) + 1;
    }

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
}






void draw_objects(void)
{
    // Prepare vertex and color data from tri_vec
    std::vector<custom_math::vertex_3> positions;
    std::vector<custom_math::vertex_3> colors;

    //for (size_t i = 0; i < voxel_centres.size(); i++)
    //{
    //    if (voxel_densities[i] > 0)
    //    {
    //        positions.push_back(voxel_centres[i]);
    //        colors.push_back(custom_math::vertex_3(1, 1, 1));
    //    }
    //}



    //draw_points(positions, colors, model_matrix);





    //positions.clear();
    //colors.clear();

    //for (size_t i = 0; i < background_centres.size(); i++)
    //{
    //    if (background_densities[i] > 0)
    //    {
    //        positions.push_back(background_centres[i]);
    //        colors.push_back(custom_math::vertex_3(1, 0.5, 0));
    //    }
    //}

    //draw_points(positions, colors, glm::mat4(1.0f));



    positions.clear();
    colors.clear();

    for (size_t i = 0; i < background_surface_centres.size(); i++)
    {
        positions.push_back(background_surface_centres[i]);
        colors.push_back(custom_math::vertex_3(0, 1, 1)); // Use a distinct color like cyan
    }

    draw_points(positions, colors, glm::mat4(1.0f));




	if (draw_triangles_on_screen)
	{
		positions.clear();
		colors.clear();

		for (const auto& tri : tri_vec)
		{
			for (size_t j = 0; j < 3; ++j)
			{
				positions.push_back(tri.vertex[j]);
				colors.push_back(tri.colour);
			}
		}

		//glm::mat4 model = glm::mat4(1.0f);
		//model = glm::rotate(model, u, glm::vec3(0.0f, 1.0f, 0.0f));
		//model = glm::rotate(model, v, glm::vec3(1.0f, 0.0f, 0.0f));

		// Draw triangles
		draw_triangles(positions, colors, model_matrix);
	}


    // Optionally draw axes as lines
    if (draw_axis) {
        std::vector<custom_math::vertex_3> axis_positions = {
            {0.0f, 0.0f, 0.0f}, {10.0f, 0.0f, 0.0f},  // x-axis
            {0.0f, 0.0f, 0.0f}, {0.0f, 10.0f, 0.0f},  // y-axis
            {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 10.0f},  // z-axis
            {0.0f, 0.0f, 0.0f}, {-10.0f, 0.0f, 0.0f}, // -x-axis
            {0.0f, 0.0f, 0.0f}, {0.0f, -10.0f, 0.0f}, // -y-axis
            {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -10.0f}  // -z-axis
        };
        std::vector<custom_math::vertex_3> axis_colors = {
            {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, // x-axis (red)
            {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, // y-axis (green)
            {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, // z-axis (blue)
            {0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, // -x-axis (gray)
            {0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, // -y-axis (gray)
            {0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}  // -z-axis (gray)
        };

        draw_lines(axis_positions, axis_colors, glm::mat4(1.0f));
    }
}







void display_func(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw objects using modern OpenGL
    draw_objects();

    // Draw UI text if enabled
    if (true == draw_control_list)
    {
        // Set text color
        glColor3d(control_list_colour.x, control_list_colour.y, control_list_colour.z);

        size_t break_size = 22;
        size_t start = 20;
        ostringstream oss;

        render_string(10, static_cast<int>(start), GLUT_BITMAP_HELVETICA_18, string("Mouse controls:"));
        render_string(10, static_cast<int>(start + 1 * break_size), GLUT_BITMAP_HELVETICA_18, string("  LMB + drag: Rotate camera"));
        render_string(10, static_cast<int>(start + 2 * break_size), GLUT_BITMAP_HELVETICA_18, string("  RMB + drag: Zoom camera"));

        render_string(10, static_cast<int>(start + 4 * break_size), GLUT_BITMAP_HELVETICA_18, string("Keyboard controls:"));
        render_string(10, static_cast<int>(start + 5 * break_size), GLUT_BITMAP_HELVETICA_18, string("  w: Draw axis"));
        render_string(10, static_cast<int>(start + 6 * break_size), GLUT_BITMAP_HELVETICA_18, string("  e: Draw text"));
        render_string(10, static_cast<int>(start + 7 * break_size), GLUT_BITMAP_HELVETICA_18, string("  u: Rotate camera +u"));
        render_string(10, static_cast<int>(start + 8 * break_size), GLUT_BITMAP_HELVETICA_18, string("  i: Rotate camera -u"));
        render_string(10, static_cast<int>(start + 9 * break_size), GLUT_BITMAP_HELVETICA_18, string("  o: Rotate camera +v"));
        render_string(10, static_cast<int>(start + 10 * break_size), GLUT_BITMAP_HELVETICA_18, string("  p: Rotate camera -v"));

        vec3 eye = main_camera.eye;
        vec3 eye_norm = normalize(eye);

        oss.clear();
        oss.str("");
        oss << "Camera position: " << eye.x << ' ' << eye.y << ' ' << eye.z;
        render_string(10, static_cast<int>(win_y - 2 * break_size), GLUT_BITMAP_HELVETICA_18, oss.str());

        oss.clear();
        oss.str("");
        oss << "Camera position (normalized): " << eye_norm.x << ' ' << eye_norm.y << ' ' << eye_norm.z;
        render_string(10, static_cast<int>(win_y - break_size), GLUT_BITMAP_HELVETICA_18, oss.str());
    }

    glFlush();
    glutSwapBuffers();
}

void keyboard_func(unsigned char key, int x, int y)
{
    switch (tolower(key))
    {
    case 't':
    {
        draw_triangles_on_screen = !draw_triangles_on_screen;
        break;
    }

    case 'w':
    {
        draw_axis = !draw_axis;
        break;
    }
    case 'e':
    {
        draw_control_list = !draw_control_list;
        break;
    }
    case 'o':
    {
        u += 0.1f;

        model_matrix = glm::mat4(1.0f);
        model_matrix = glm::rotate(model_matrix, u, glm::vec3(0.0f, 1.0f, 0.0f));
        model_matrix = glm::rotate(model_matrix, v, glm::vec3(1.0f, 0.0f, 0.0f));

        get_triangles("chr_knight.vox", tri_vec);
        voxel_grid.initialize(voxel_centres, voxel_densities);
        get_background_points();
        get_surface_points();
        break;
    }
    case 'p':
    {
        u -= 0.1f;

        model_matrix = glm::mat4(1.0f);
        model_matrix = glm::rotate(model_matrix, u, glm::vec3(0.0f, 1.0f, 0.0f));
        model_matrix = glm::rotate(model_matrix, v, glm::vec3(1.0f, 0.0f, 0.0f));

        get_triangles("chr_knight.vox", tri_vec);
        voxel_grid.initialize(voxel_centres, voxel_densities);
        get_background_points();
        get_surface_points();
        break;
    }
    case 'k':
    {
        v += 0.1f;

        model_matrix = glm::mat4(1.0f);
        model_matrix = glm::rotate(model_matrix, u, glm::vec3(0.0f, 1.0f, 0.0f));
        model_matrix = glm::rotate(model_matrix, v, glm::vec3(1.0f, 0.0f, 0.0f));

        get_triangles("chr_knight.vox", tri_vec);
        voxel_grid.initialize(voxel_centres, voxel_densities);
        get_background_points();
        get_surface_points();
        break;
    }
    case 'l':
    {
        v -= 0.1f;

        model_matrix = glm::mat4(1.0f);
        model_matrix = glm::rotate(model_matrix, u, glm::vec3(0.0f, 1.0f, 0.0f));
        model_matrix = glm::rotate(model_matrix, v, glm::vec3(1.0f, 0.0f, 0.0f));

        get_triangles("chr_knight.vox", tri_vec);
        voxel_grid.initialize(voxel_centres, voxel_densities);
        get_background_points();
        get_surface_points();
        break;
    }

    default:
        break;
    }
}

void mouse_func(int button, int state, int x, int y)
{
    if (GLUT_LEFT_BUTTON == button)
    {
        if (GLUT_DOWN == state)
            lmb_down = true;
        else
            lmb_down = false;
    }
    else if (GLUT_MIDDLE_BUTTON == button)
    {
        if (GLUT_DOWN == state)
            mmb_down = true;
        else
            mmb_down = false;
    }
    else if (GLUT_RIGHT_BUTTON == button)
    {
        if (GLUT_DOWN == state)
            rmb_down = true;
        else
            rmb_down = false;
    }
}

void motion_func(int x, int y)
{
    int prev_mouse_x = mouse_x;
    int prev_mouse_y = mouse_y;

    mouse_x = x;
    mouse_y = y;

    int mouse_delta_x = mouse_x - prev_mouse_x;
    int mouse_delta_y = prev_mouse_y - mouse_y;

    if (true == lmb_down && (0 != mouse_delta_x || 0 != mouse_delta_y))
    {
        main_camera.u -= static_cast<float>(mouse_delta_y) * u_spacer;
        main_camera.v += static_cast<float>(mouse_delta_x) * v_spacer;
    }
    else if (true == rmb_down && (0 != mouse_delta_y))
    {
        main_camera.w -= static_cast<float>(mouse_delta_y) * w_spacer;

        if (main_camera.w < 1.1f)
            main_camera.w = 1.1f;
    }

    main_camera.calculate_camera_matrices(win_x, win_y);
}

void passive_motion_func(int x, int y)
{
    mouse_x = x;
    mouse_y = y;
}

void cleanup(void)
{





    glutDestroyWindow(win_id);
}