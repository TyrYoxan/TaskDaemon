cmake_minimum_required(VERSION 3.10)

project(TaskDaemon)

# Ajout du fichier source principal
#add_executable(main main.c)

# Ajout des fichiers sources des exécutables receiver et sender
#add_executable(receiver receiver.c)
#add_executable(sender sender.c)

#Ajout des fichiers sources des executable taskd et taskcli
add_executable(taskd taskd.c)
add_executable(taskcli taskcli.c)

# Ajout du fichier source de la bibliothèque dynamique
add_library(message SHARED message.c)

# Inclusion du répertoire contenant le fichier d'en-tête de la bibliothèque dynamique
target_include_directories(message PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Liaison des exécutables avec la bibliothèque dynamique
#target_link_libraries(receiver message)
#target_link_libraries(sender message)
target_link_libraries(taskd message)
target_link_libraries(taskcli message)
