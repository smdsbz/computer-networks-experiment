#include "pch.h"

#include <iostream>
#include "Server.h"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Text_Display.H>
#include <FL/fl_ask.H>


/* Session Data */

volatile bool server_running = false;
CWinThread *pmtrd = 0;
Server *ps = 0;

/* Funtionalities */

Fl_Input *address_input = 0;
Fl_Input *port_input = 0;
Fl_Box *static_dir_display = 0;
Fl_Text_Buffer *route_output = 0;

void address_input_change_cb(Fl_Widget *w, void *data)
{
    Fl_Input *wi = (Fl_Input *)w;
    if (ps != 0) {
        fl_message(
            "Cannot change server configuration on the fly!"
        );
        return;
    }
    fl_message(
        (
            std::string("Server address changed to ")
            + std::string(wi->value())
        ).c_str()
    );
    return;
}


void port_input_change_cb(Fl_Widget *w, void *data)
{
    Fl_Input *wi = (Fl_Input *)w;
    if (ps != 0) {
        fl_message(
            "Cannot change server configuration on the fly!"
        );
        return;
    }
    fl_message(
        (
            std::string("Server port changed to ")
            + std::string(wi->value())
        ).c_str()
    );
    return;
}


void start_button_cb(Fl_Widget *w, void *data)
{
    // check address legality
    // TODO:
    // check port legality
    // TODO:
    // run server (non-blocking)
    if (ps != 0) {
        fl_message(
            "Server already running!"
        );
        return;
    }

    try {
        ps = new Server(
            server_running,
            static_dir_display->label(),
            address_input->value(),
            port_input->value(),
            ::route_output
        );
    }
    catch (std::runtime_error &e) {
        fl_message(
            "Server could not be initialzied!"
        );
    }
    server_running = true;
    pmtrd = AfxBeginThread(
        Server::StartServiceLoop,
        (LPVOID)ps
    );
    if (pmtrd == 0) {
        fl_message(
            "Service loop cannot be started!"
        );
        server_running = false;
        delete ps; ps = 0;
    }
    pmtrd->m_bAutoDelete = FALSE;
    fl_message(
        "Server started successfully!"
    );
    return;
}


void stop_button_cb(Fl_Widget *w, void *data)
{
    if (server_running == false) {
        fl_message(
            "No server running!"
        );
        return;
    }
    server_running = false;
    DWORD wait_result;
    do {
        wait_result = WaitForSingleObject(
            pmtrd->m_hThread,
            INFINITE
        );
    } while (wait_result != WAIT_OBJECT_0);
    std::cout << "Service loop exited!" << std::endl;
    pmtrd = 0;
    ps->StopServer();
    delete ps;
    ps = 0;
    fl_message(
        "Server shutdown!"
    );
    return;
}


void static_dir_chooser_cb(Fl_Widget *w, void *data)
{
    if (server_running) {
        fl_message(
            "Cannot change server settings on the fly!"
        );
        return;
    }
    char *new_dir = fl_dir_chooser(
        "Set server static path",
        static_dir_display->label(),
        0
    );
    if (new_dir != 0) {
        // delete last '/' is present
        std::string server_static_directory = std::string(new_dir);
        if (server_static_directory[server_static_directory.size() - 1] == '/') {
            server_static_directory = server_static_directory.substr(
                0,
                server_static_directory.size() - 1
            );
        }
        ::static_dir_display->copy_label(server_static_directory.c_str());
    }
    fl_message(
        (
            std::string("Server static path set to ")
            + std::string(static_dir_display->label())
        ).c_str()
    );
    return;
}


/* Shape UI */

const unsigned left_margin = 10;
const unsigned input_offset = 70;
const unsigned line_height = 30;

int main(void) {
    // Create window
    Fl_Window *main_window = new Fl_Window(
        480,
        600
    );
    unsigned y_til = 10;

    Fl_Input *address_input = new Fl_Input(
        left_margin + input_offset, y_til,
        300, line_height,
        "Address: "
    ); y_til += 40;
    address_input->replace(
        0, address_input->size(),
        Config_Address
    );
    address_input->callback(address_input_change_cb);
    ::address_input = address_input;

    Fl_Input *port_input = new Fl_Input(
        left_margin + input_offset, y_til,
        300, line_height,
        "Port: "
    ); y_til += 40;
    port_input->replace(
        0, port_input->size(),
        Config_Port
    );
    port_input->callback(port_input_change_cb);
    ::port_input = port_input;
    
    y_til += 10;
    Fl_Box *static_dir_display_label = new Fl_Box(
        left_margin, y_til,
        input_offset, line_height,
        "Static Path: "
    );
    Fl_Box *static_dir_display = new Fl_Box(
        left_margin + input_offset, y_til,
        200, line_height,
        Config_StaticRootPath.c_str()
    );
    ::static_dir_display = static_dir_display;
    Fl_Button *static_dir_chooser = new Fl_Button(
        left_margin + 210 + input_offset, y_til,
        100, line_height,
        "Choose path"
    ); y_til += 40;
    static_dir_chooser->callback(static_dir_chooser_cb);

    y_til += 10;
    Fl_Button *start_button = new Fl_Button(
        left_margin + 100, y_til,
        100, line_height,
        "Start"
    );
    start_button->callback(start_button_cb);

    Fl_Button *stop_button = new Fl_Button(
        left_margin + 200 + 10, y_til,
        100, line_height,
        "Stop"
    ); y_til += 40;
    stop_button->callback(stop_button_cb);

    y_til += 20;
    Fl_Text_Buffer *route_output = new Fl_Text_Buffer();
    Fl_Text_Display *route_output_display = new Fl_Text_Display(
        left_margin, y_til,
        480 - 2 * left_margin, 600 - y_til - 10
    );
    route_output_display->buffer(route_output);
    ::route_output = route_output;

    main_window->end();
    main_window->show();
    int guiret = Fl::run();

    // shutdown server
    if (server_running == true) {
        server_running = false;
        DWORD wait_result;
        do {
            wait_result = WaitForSingleObject(
                pmtrd->m_hThread,
                INFINITE
            );
        } while (wait_result != WAIT_OBJECT_0);
        std::cout << "Service loop exited!" << std::endl;
        pmtrd = 0;
        ps->StopServer();
        delete ps;
        ps = 0;
    }

    return guiret;
}
