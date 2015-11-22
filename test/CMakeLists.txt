add_executable(play test.cpp)
target_link_libraries(play ${OPENSCENEGRAPH_LIBRARIES} ${OpenCV_LIBS} Qt5::Widgets)

add_executable(play2 test_mvp.cpp)
target_link_libraries(play2 ${OPENSCENEGRAPH_LIBRARIES} ${OpenCV_LIBS} Qt5::Widgets)

add_executable(play3 test_mvp2.cpp)
target_link_libraries(play3 ${OPENSCENEGRAPH_LIBRARIES} ${OpenCV_LIBS} Qt5::Widgets)

add_executable(play_array array.cpp)
target_link_libraries(play_array ${OPENSCENEGRAPH_LIBRARIES} ${OpenCV_LIBS} Qt5::Widgets)

add_executable(play_frustum test2.cpp)
target_link_libraries(play_frustum ${OPENSCENEGRAPH_LIBRARIES} ${OpenCV_LIBS} Qt5::Widgets)

add_executable(play_window testMouse.cpp)
target_link_libraries(play_window ${OPENSCENEGRAPH_LIBRARIES} ${OpenCV_LIBS} Qt5::Widgets)