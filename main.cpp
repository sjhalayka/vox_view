#include "main.h"
#include "shader_utils.h"

// Shader source code
const char* vertexShaderSource = R"(
#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fragColor;

void main()
{
    fragColor = color;
    gl_Position = projection * view * model * vec4(position, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 430 core

in vec3 fragColor;
out vec4 finalColor;

void main()
{
    finalColor = vec4(fragColor, 1.0);
}
)";

// Axis shader source code
const char* axisVertexShaderSource = R"(
#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

uniform mat4 view;
uniform mat4 projection;

out vec3 fragColor;

void main()
{
    fragColor = color;
    gl_Position = projection * view * vec4(position, 1.0);
}
)";

const char* axisFragmentShaderSource = R"(
#version 430 core

in vec3 fragColor;
out vec4 finalColor;

void main()
{
    finalColor = vec4(fragColor, 1.0);
}
)";

// OpenGL 4 variables
GLuint axis_shader_program = 0;

int main(int argc, char** argv)
{
    cout << setprecision(20) << endl;

    read_quads_from_vox_file("chr_knight.vox", tri_vec);

    glutInit(&argc, argv);
    init_opengl(win_x, win_y);

    // Initialize GLEW after GLUT and context creation
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        cerr << "Error: " << glewGetErrorString(err) << endl;
        return 1;
    }

    // Setup OpenGL 4 objects
    init_shaders();
    setup_vao();

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

void init_shaders(void)
{
    // Create the main shader program
    shader_program = createShaderProgram(vertexShaderSource, fragmentShaderSource);

    // Get uniform locations
    model_loc = glGetUniformLocation(shader_program, "model");
    view_loc = glGetUniformLocation(shader_program, "view");
    proj_loc = glGetUniformLocation(shader_program, "projection");

    // Create the axis shader program
    axis_shader_program = createShaderProgram(axisVertexShaderSource, axisFragmentShaderSource);
}

void setup_vao(void)
{
    // Create buffers for the main mesh
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;

    // Convert triangle data to buffers
    size_t index_offset = 0;
    for (size_t i = 0; i < tri_vec.size(); i++) {
        for (size_t j = 0; j < 3; j++) {
            Vertex vertex;
            vertex.position[0] = tri_vec[i].vertex[j].x;
            vertex.position[1] = tri_vec[i].vertex[j].y;
            vertex.position[2] = tri_vec[i].vertex[j].z;
            vertex.color[0] = tri_vec[i].colour.x;
            vertex.color[1] = tri_vec[i].colour.y;
            vertex.color[2] = tri_vec[i].colour.z;
            vertices.push_back(vertex);
        }

        // Add indices for triangle
        indices.push_back(static_cast<GLuint>(index_offset));
        indices.push_back(static_cast<GLuint>(index_offset + 1));
        indices.push_back(static_cast<GLuint>(index_offset + 2));
        index_offset += 3;
    }

    // Create and bind the main VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create and bind the VBO
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    // Create and bind the EBO
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    // Set attribute pointers
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind VAO
    glBindVertexArray(0);

    // Setup axis VAO
    float axis_vertices[] = {
        // Position (x,y,z)     // Color (r,g,b)
        0.0f, 0.0f, 0.0f,      1.0f, 0.0f, 0.0f,  // x-axis start (red)
        10.0f, 0.0f, 0.0f,      1.0f, 0.0f, 0.0f,  // x-axis end
        0.0f, 0.0f, 0.0f,      0.0f, 1.0f, 0.0f,  // y-axis start (green)
        0.0f, 10.0f, 0.0f,      0.0f, 1.0f, 0.0f,  // y-axis end
        0.0f, 0.0f, 0.0f,      0.0f, 0.0f, 1.0f,  // z-axis start (blue)
        0.0f, 0.0f, 10.0f,      0.0f, 0.0f, 1.0f,  // z-axis end
        0.0f, 0.0f, 0.0f,      0.5f, 0.5f, 0.5f,  // -x-axis start (gray)
        -10.0f, 0.0f, 0.0f,     0.5f, 0.5f, 0.5f,  // -x-axis end
        0.0f, 0.0f, 0.0f,      0.5f, 0.5f, 0.5f,  // -y-axis start
        0.0f, -10.0f, 0.0f,     0.5f, 0.5f, 0.5f,  // -y-axis end
        0.0f, 0.0f, 0.0f,      0.5f, 0.5f, 0.5f,  // -z-axis start
        0.0f, 0.0f, -10.0f,     0.5f, 0.5f, 0.5f   // -z-axis end
    };

    // Generate axis VAO and VBO
    glGenVertexArrays(1, &axis_vao);
    glBindVertexArray(axis_vao);

    glGenBuffers(1, &axis_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, axis_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axis_vertices), axis_vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind axis VAO
    glBindVertexArray(0);
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
    // Draw the model using modern OpenGL
    glUseProgram(shader_program);

    // Set uniforms
    glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model_matrix));
    glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(main_camera.view_mat));
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, glm::value_ptr(main_camera.projection_mat));

    // Draw the mesh
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(tri_vec.size() * 3), GL_UNSIGNED_INT, 0);

    // Draw coordinate axes if enabled
    if (draw_axis)
    {
        glUseProgram(axis_shader_program);

        GLint axis_view_loc = glGetUniformLocation(axis_shader_program, "view");
        GLint axis_proj_loc = glGetUniformLocation(axis_shader_program, "projection");

        glUniformMatrix4fv(axis_view_loc, 1, GL_FALSE, glm::value_ptr(main_camera.view_mat));
        glUniformMatrix4fv(axis_proj_loc, 1, GL_FALSE, glm::value_ptr(main_camera.projection_mat));

        glBindVertexArray(axis_vao);
        glDrawArrays(GL_LINES, 0, 12); // 6 axes, 2 points each
    }

    // Reset state
    glBindVertexArray(0);
    glUseProgram(0);
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
    case 'u':
    {
        main_camera.u -= u_spacer;
        main_camera.calculate_camera_matrices(win_x, win_y);
        break;
    }
    case 'i':
    {
        main_camera.u += u_spacer;
        main_camera.calculate_camera_matrices(win_x, win_y);
        break;
    }
    case 'o':
    {
        main_camera.v -= v_spacer;
        main_camera.calculate_camera_matrices(win_x, win_y);
        break;
    }
    case 'p':
    {
        main_camera.v += v_spacer;
        main_camera.calculate_camera_matrices(win_x, win_y);
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
    // Delete OpenGL objects
    glDeleteVertexArrays(1, &vao);
    glDeleteVertexArrays(1, &axis_vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &axis_vbo);
    glDeleteProgram(shader_program);
    glDeleteProgram(axis_shader_program);

    glutDestroyWindow(win_id);
}