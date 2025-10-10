#include <iostream>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

const char* SOCKET_PATH = "/tmp/kllm.sock";

// Function to send a request to the daemon and receive a response
bool get_inference(const std::string& prompt, std::string& response) {
    int client_fd;
    struct sockaddr_un addr;

    if ((client_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("client: socket failed");
        return false;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("client: connect failed");
        close(client_fd);
        return false;
    }

    // Send the prompt
    std::string request = prompt + "\n";
    write(client_fd, request.c_str(), request.length());

    // Read the response
    char buffer[4096] = {0};
    read(client_fd, buffer, 4095);
    response = buffer;

    close(client_fd);
    return true;
}


int main() {
    std::cout << "--- KLLM Client ---" << std::endl;
    std::cout << "Type 'exit' to quit." << std::endl;
    std::cout << "------------------------------------" << std::endl;

    std::string input;
    while (true) {
        std::cout << "\n### Human: ";
        std::getline(std::cin, input);

        if (input == "exit") {
            break;
        }

        std::string response;
        if (get_inference(input, response)) {
            std::cout << "### Assistant: " << response;
        } else {
            std::cerr << "Error: Failed to communicate with the KLLM daemon." << std::endl;
            std::cerr << "Please ensure the daemon is running." << std::endl;
        }
    }

    std::cout << "\n--- Exiting client ---" << std::endl;
    return 0;
}