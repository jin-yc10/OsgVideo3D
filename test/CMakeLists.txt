add_executable(play test.cpp)
target_link_libraries(play ${OPENSCENEGRAPH_LIBRARIES} ${OpenCV_LIBS} Qt5::Widgets)

add_executable(play2 test_mvp.cpp)
target_link_libraries(play2 ${OPENSCENEGRAPH_LIBRARIES} ${OpenCV_LIBS} Qt5::Widgets)