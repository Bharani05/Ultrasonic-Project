ji#include<stdio.h>
#include<stdlib.h>

struct node
{
    int data;
    struct node *next;
};
void addLast(struct node **head,int val)
{
    struct node *newNode = malloc(sizeof(struct node));
    newNode ->data = val;
    newNode ->next = NULL;
    if (*head == NULL)
    *head = newNode;
    else
    {
        struct node *Lastnode = *head;
        while(Lastnode->next !=NULL)
        {

            Lastnode = Lastnode->next;
        }
        Lastnode -> next = newNode;
    }
}
void printlist(struct node *head)
{
    struct node *temp = head;
    while(temp != NULL)
    {
        printf("%d->",temp -> data);
        temp = temp -> next;
    }
    printf("NULL\n");
}
int main()
{
    struct node *head = NULL;
    addLast(&head,13);
    addLast(&head,34);
    addLast(&head,67);

         printlist(head);
         return 0;
}

CMakeLists.txt
cmake_minimum_required(VERSION 3.30.0)

project(hello_cmake)

add_executable(hello_cmake hello_cmake.c)

target_link_libraries(hello_cmake PUBLIC some_library)

 cannot find -lsome_library: No such file or directory

link_directories(/path/to/some_library)
target_link_libraries(hello_cmake PUBLIC some_library)


find_library(SOME_LIBRARY_PATH some_library PATHS /custom/path)
if(SOME_LIBRARY_PATH)
    target_link_libraries(hello_cmake PUBLIC ${SOME_LIBRARY_PATH})
else()
    message(FATAL_ERROR "some_library not found")
endif()


./cmake/src/CMakeLists.txt
cmake_minimum_required(VERSION 3.30.0)

project(hello)

add_library(simplecmake printhello.c)

target_include_directories(simplecmake PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)

 ./cmake/example/CMakeLists.txt
project(hello)

add_executable(link link.c)

target_link_libraries(link PRIVATE simplecmake)

./cmake/CMakeLists.txt

cmake_minimum_required(VERSION 3.30.0)

project(helo)


add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(example)

option(BUILD_EXAMPLES "wheather or not build examples" ON)

if(BUILD_EXAMPLE)
        message(STATUS "Building example! ...")
        add_subdirectory(example)

endif()

error: pathspec 'v4.4.0' did not match any file(s) known to git

git ls-remote --tags origin
git ls-remote --heads origin


