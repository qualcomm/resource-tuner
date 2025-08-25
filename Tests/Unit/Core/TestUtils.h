#ifndef UNIT_TEST_UTILS
#define UNIT_TEST_UTILS

#define RUN_TEST(test)                                              \
do {                                                                \
    std::cout<<"Running Test: "<<#test<<std::endl;                  \
    test();                                                         \
    std::cout<<#test<<": Run Successful"<<std::endl;                \
    std::cout<<"-------------------------------------"<<std::endl;  \
} while(false);                                                     \

#define C_ASSERT(cond)                                                                          \
    if(cond == false) {                                                                         \
        std::cerr<<"Assertion failed at line [" << __LINE__ << "]: "<<#cond<<std::endl;         \
        std::cerr<<"Test: ["<<__func__<<"] Failed, Terminating Suite\n"<<std::endl;             \
        exit(EXIT_FAILURE);                                                                     \
    }                                                                                           \

#define C_ASSERT_NEAR(val1, val2, tol)                                               \
    if (std::fabs((val1) - (val2)) > (tol)) {                                        \
        std::cerr<<"Condition Check on line:["<<__LINE__<<"]  failed"<<std::endl;    \
        std::cerr<<"Test: ["<<__func__<<"] Failed, Terminating Suite\n"<<std::endl;  \
        exit(EXIT_FAILURE);                                                          \
    }                                                                                \

#endif
