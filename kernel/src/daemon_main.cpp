#include "pseudo_kernel.h"
#include <iostream>
#include <string>
#include <vector>
#include <csignal>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

const char* SOCKET_PATH = "/tmp/kllm.sock";
PseudoKernel g_kernel;

void signal_handler(int signum) {
    std::cout << "\nCaught signal " << signum << ", shutting down gracefully." << std::endl;
    unlink(SOCKET_PATH);
    exit(signum);
}

int main() {
    // Register signal handlers for graceful shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // 1. Initialize the kernel and load the model
    if (g_kernel.initialize() != KLLM_SUCCESS) {
        std::cerr << "Daemon Error: Failed to initialize kernel." << std::endl;
        return 1;
    }
    // Note: This path is relative to the build directory.
    if (g_kernel.load_model("../models/tinyllama-2-1b-miniguanaco.Q4_K_M.gguf") != KLLM_SUCCESS) {
        std::cerr << "Daemon Error: Failed to load model." << std::endl;
        g_kernel.shutdown();
        return 1;
    }

    // 2. Set up the Unix socket
    int server_fd;
    struct sockaddr_un addr;

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return 1;
    }

    // Make sure the socket file doesn't already exist
    unlink(SOCKET_PATH);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen failed");
        close(server_fd);
        return 1;
    }

    std::cout << "Daemon: Listening on " << SOCKET_PATH << std::endl;

    // 3. Main accept loop (request handling logic will be added in the next step)
    while (true) {
        int client_socket;
        struct sockaddr_un client_addr;
        socklen_t client_len = sizeof(client_addr);

        std::cout << "Daemon: Waiting for a connection..." << std::endl;
        if ((client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &client_len)) < 0) {
            perror("accept failed");
            continue; // Continue to the next iteration
        }

        std::cout << "Daemon: Client connected." << std::endl;

        // 4. Handle the request
        char buffer[4096] = {0};
        read(client_socket, buffer, 4095);
        std::string input_prompt(buffer);

        // Remove newline characters from the prompt
        input_prompt.erase(std::remove(input_prompt.begin(), input_prompt.end(), '\n'), input_prompt.end());

        std::cout << "Daemon: Received prompt: \"" << input_prompt << "\"" << std::endl;

        std::string response_text;
        kllm_status_t status = g_kernel.run_inference(input_prompt, response_text);

        if (status != KLLM_SUCCESS) {
            response_text = "ERROR: Inference failed.\n";
        } else {
            response_text += "\n";
        }

        // Send the response back to the client
        write(client_socket, response_text.c_str(), response_text.length());

        close(client_socket);
        std::cout << "Daemon: Client disconnected." << std::endl;
    }

    // Cleanup
    close(server_fd);
    unlink(SOCKET_PATH);
    g_kernel.shutdown();

    return 0;
}