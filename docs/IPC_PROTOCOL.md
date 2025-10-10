# KLLM Inter-Process Communication (IPC) Protocol

This document defines the simple, text-based protocol used for communication between the `kllm_daemon` and its clients over a Unix socket.

## 1. Overview

The communication is based on a request-response model. The client connects to the daemon, sends a request, and waits for a response. The connection is closed after each transaction.

The protocol is text-based and uses newline characters (`\n`) to delimit messages.

## 2. Socket Path

The daemon will create and listen on a Unix socket located at:
`/tmp/kllm.sock`

## 3. Communication Flow

1.  **Client Connects:** The client initiates a connection to the daemon's Unix socket.
2.  **Client Sends Request:** The client sends the inference prompt as a single line of text, terminated by a newline character.
    *   Example: `What is the capital of France?\n`
3.  **Daemon Processes Request:** The daemon reads the prompt, passes it to the `PseudoKernel` for inference.
4.  **Daemon Sends Response:** The daemon sends the model's output back to the client as a single line of text, terminated by a newline character.
    *   Example: `The capital of France is Paris.\n`
5.  **Connection Closed:** The daemon closes the connection. The client reads the response and then closes its end of the connection.

## 4. Error Handling

If the daemon encounters an error during inference, it will send a specific error message to the client, prefixed with `ERROR:`.

*   Example: `ERROR: Model not loaded.\n`

This simple protocol ensures that both the client and daemon can communicate reliably without the need for complex parsing logic.