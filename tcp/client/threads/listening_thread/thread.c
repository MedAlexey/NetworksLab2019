#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "thread.h"
#include <stdlib.h>
#include <sys/socket.h>
#include "../../main_window/main_window.h"

#include "../../constants.h"


int readn(int sockfd, void* dst, size_t len) {
    int total_number_read = 0;
    int local_number_read = 0;

    while (len > 0) {
        local_number_read = read(sockfd, dst, len);

        if (local_number_read > 0) {
            total_number_read += local_number_read;
            len -= local_number_read;
        } else if (local_number_read == 0) {
            return total_number_read;
        } else {
            return -1;
        }

    }

    return total_number_read;
}


// сдвиг элементов массива влево
void arrayRotateLeft(char* array[], int size) {
    free(array[0]);
    for (int i = 0; i < size - 1; i++) {
        array[i] = array[i + 1];
    }
}


void array_append(char *dst[], int dst_size, char src[]) {
    arrayRotateLeft(dst, dst_size);
    dst[dst_size-1] = strdup(src);
}


void init_array(char* arr[], int arr_size) {
    for (int i = 0; i < arr_size; ++i) {
        arr[i] = strdup("");
    }
}


// listen to socket and output incoming messages
void* listening_thread(void* arg) {
    int number_read;
    char message[MESSAGE_SIZE];
    char* shown_messages[((Listening_thread_input*) arg) -> output_window_height];

    init_array(shown_messages, ((Listening_thread_input*) arg) -> output_window_height);


    while (1) {
        bzero(message, MESSAGE_SIZE);
        int message_size;
        //reading message size
        readn(((Listening_thread_input *) arg)-> sockfd, &message_size, sizeof(int));
        // reading message
        number_read = readn(((Listening_thread_input *) arg)-> sockfd, message,  message_size);

        if (number_read < 0) {
            fprintf(stderr, "ERROR reading in listening thread\n");
        }

        if (number_read == 0) {
            main_window_exit();
            shutdown( ((Listening_thread_input*) arg) -> sockfd, SHUT_RDWR);
            close( ((Listening_thread_input*) arg) -> sockfd );
            fprintf(stderr,"ERROR: Server disconnected.\n");
            pthread_exit((void*) EXIT_FAILURE);
        }

        array_append(shown_messages, ((Listening_thread_input*) arg) -> output_window_height, message);
        main_window_write_to_output_window(shown_messages, ((Listening_thread_input*) arg) -> output_window_height);
    }

}


Listening_thread_input* listening_thread_init_input_structure(int sockfd, int output_window_height) {
    Listening_thread_input* new_input_structure = (Listening_thread_input*) malloc(sizeof(Listening_thread_input));

    new_input_structure -> sockfd = sockfd;
    new_input_structure -> output_window_height = output_window_height;

    return new_input_structure;
}


pthread_t create_listening_thread(int sockfd, int output_window_height) {
    pthread_t listnening_thread;
    Listening_thread_input* listening_thread_input = listening_thread_init_input_structure(sockfd, output_window_height);

    if( pthread_create(&listnening_thread, NULL, listening_thread, (void *) listening_thread_input)) {
        fprintf(stderr, "ERROR creating listening thread\n");
    }

    return listnening_thread;
}