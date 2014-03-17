
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <sstream>
#include <string>

#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <boost/chrono/system_clocks.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/utility.hpp>

#include <boost/fiber/all.hpp>

bool value1 = false;

class copyable
{
public:
    bool    state;

    copyable() :
        state( false)
    {}

    copyable( int) :
        state( true)
    {}

    void operator()()
    { value1 = state; }
};

class moveable
{
private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE( moveable);

public:
    bool    state;

    moveable() :
        state( false)
    {}

    moveable( int) :
        state( true)
    {}

    moveable( BOOST_RV_REF( moveable) other) :
        state( false)
    { std::swap( state, other.state); }

    moveable & operator=( BOOST_RV_REF( moveable) other)
    {
        if ( this == & other) return * this;
        moveable tmp( boost::move( other) );
        std::swap( state, tmp.state);
        return * this;
    }

    void operator()()
    { value1 = state; }
};

void f1() {}

void f2()
{
    boost::this_fiber::yield();
}

void f4()
{
    boost::fibers::fiber s( f2);
    BOOST_CHECK( s);
    BOOST_CHECK( s.joinable() );
    s.join();
    BOOST_CHECK( ! s);
    BOOST_CHECK( ! s.joinable() );
}

void f6( int & i)
{
    i = 1;
    boost::this_fiber::yield();
    i = 1;
    boost::this_fiber::yield();
    i = 2;
    boost::this_fiber::yield();
    i = 3;
    boost::this_fiber::yield();
    i = 5;
    boost::this_fiber::yield();
    i = 8;
}

void f7( int & i, bool & failed)
{
    try
    {
        i = 1;
        boost::this_fiber::yield();
        boost::this_fiber::interruption_point();
        i = 1;
        boost::this_fiber::yield();
        boost::this_fiber::interruption_point();
        i = 2;
        boost::this_fiber::yield();
        boost::this_fiber::interruption_point();
        i = 3;
        boost::this_fiber::yield();
        boost::this_fiber::interruption_point();
        i = 5;
        boost::this_fiber::yield();
        boost::this_fiber::interruption_point();
        i = 8;
    }
    catch ( boost::fibers::fiber_interrupted const&)
    { failed = true; }
}

void interruption_point_wait(boost::fibers::mutex* m,bool* failed)
{
    boost::unique_lock<boost::fibers::mutex> lk(*m);
    boost::this_fiber::interruption_point();
    *failed=true;
}

void disabled_interruption_point_wait(boost::fibers::mutex* m,bool* failed)
{
    boost::unique_lock<boost::fibers::mutex> lk(*m);
    boost::this_fiber::disable_interruption dc;
    boost::this_fiber::interruption_point();
    *failed=false;
}

void interruption_point_join( boost::fibers::fiber & f)
{
    f.join();
}

void test_move()
{
    {
        boost::fibers::fiber s1;
        BOOST_CHECK( ! s1);
        boost::fibers::fiber s2( f2);
        BOOST_CHECK( s2);
        s1 = boost::move( s2);
        BOOST_CHECK( s1);
        BOOST_CHECK( ! s2);
        s1.join();
    }

    {
        value1 = false;
        copyable cp( 3);
        BOOST_CHECK( cp.state);
        BOOST_CHECK( ! value1);
        boost::fibers::fiber s( cp);
        s.join();
        BOOST_CHECK( cp.state);
        BOOST_CHECK( value1);
    }

    {
        value1 = false;
        moveable mv( 7);
        BOOST_CHECK( mv.state);
        BOOST_CHECK( ! value1);
        boost::fibers::fiber s( boost::move( mv) );
        s.join();
        BOOST_CHECK( ! mv.state);
        BOOST_CHECK( value1);
    }
}

void test_priority()
{
    boost::fibers::fiber f( f1);
    BOOST_CHECK_EQUAL( 0, f.priority() );
    f.priority( 7);
    BOOST_CHECK_EQUAL( 7, f.priority() );
    f.join();
}

void test_id()
{
    boost::fibers::fiber s1;
    boost::fibers::fiber s2( f2);
    BOOST_CHECK( ! s1);
    BOOST_CHECK( s2);

    BOOST_CHECK_EQUAL( boost::fibers::fiber::id(), s1.get_id() );
    BOOST_CHECK( boost::fibers::fiber::id() != s2.get_id() );

    boost::fibers::fiber s3( f1);
    BOOST_CHECK( s2.get_id() != s3.get_id() );

    s1 = boost::move( s2);
    BOOST_CHECK( s1);
    BOOST_CHECK( ! s2);

    BOOST_CHECK( boost::fibers::fiber::id() != s1.get_id() );
    BOOST_CHECK_EQUAL( boost::fibers::fiber::id(), s2.get_id() );

    BOOST_CHECK( ! s2.joinable() );

    s1.join();
    s3.join();
}

void test_detach()
{
    {
        boost::fibers::fiber s1( f1);
        BOOST_CHECK( s1);
        s1.detach();
        BOOST_CHECK( ! s1);
        BOOST_CHECK( ! s1.joinable() );
    }

    {
        boost::fibers::fiber s2( f2);
        BOOST_CHECK( s2);
        s2.detach();
        BOOST_CHECK( ! s2);
        BOOST_CHECK( ! s2.joinable() );
    }
}

void test_replace()
{
    boost::fibers::round_robin ds;
    boost::fibers::set_scheduling_algorithm( & ds);
    boost::fibers::fiber s1( f1);
    BOOST_CHECK( s1);
    boost::fibers::fiber s2( f2);
    BOOST_CHECK( s2);

    s1.join();
    s2.join();
}

void test_complete()
{
    boost::fibers::fiber s1( f1);
    BOOST_CHECK( s1);
    boost::fibers::fiber s2( f2);
    BOOST_CHECK( s2);

    s1.join();
    s2.join();
}

void test_join_in_thread()
{
    boost::fibers::fiber s( f2);
    BOOST_CHECK( s);
    BOOST_CHECK( s.joinable() );
    s.join();
    BOOST_CHECK( ! s);
    BOOST_CHECK( ! s.joinable() );
}

void test_join_and_run()
{
    boost::fibers::fiber s( f2);
    BOOST_CHECK( s);
    BOOST_CHECK( s.joinable() );
    s.join();
    BOOST_CHECK( ! s.joinable() );
    BOOST_CHECK( ! s);
}

void test_join_in_fiber()
{
    // spawn fiber s
    // s spawns an new fiber s' in its fiber-fn
    // s' yields in its fiber-fn
    // s joins s' and gets suspended (waiting on s')
    boost::fibers::fiber s( f4);
    // run() resumes s + s' which completes
    s.join();
    //BOOST_CHECK( ! s);
}

void test_yield()
{
    int v1 = 0, v2 = 0;
    BOOST_CHECK_EQUAL( 0, v1);
    BOOST_CHECK_EQUAL( 0, v2);
    boost::fibers::fiber s1( boost::bind( f6, boost::ref( v1) ) );
    boost::fibers::fiber s2( boost::bind( f6, boost::ref( v2) ) );
    s1.join();
    s2.join();
    BOOST_CHECK( ! s1);
    BOOST_CHECK( ! s2);
    BOOST_CHECK_EQUAL( 8, v1);
    BOOST_CHECK_EQUAL( 8, v2);
}

void test_fiber_interrupts_at_interruption_point()
{
    boost::fibers::mutex m;
    bool failed=false;
    bool interrupted = false;
    boost::unique_lock<boost::fibers::mutex> lk(m);
    boost::fibers::fiber f(boost::bind(&interruption_point_wait,&m,&failed));
    f.interrupt();
    lk.unlock();
    try
    { f.join(); }
    catch ( boost::fibers::fiber_interrupted const& e)
    { interrupted = true; }
    BOOST_CHECK( interrupted);
    BOOST_CHECK(!failed);
}

void test_fiber_no_interrupt_if_interrupts_disabled_at_interruption_point()
{
    boost::fibers::mutex m;
    bool failed=true;
    boost::unique_lock<boost::fibers::mutex> lk(m);
    boost::fibers::fiber f(boost::bind(&disabled_interruption_point_wait,&m,&failed));
    f.interrupt();
    lk.unlock();
    f.join();
    BOOST_CHECK(!failed);
}

void test_fiber_interrupts_at_join()
{
    int i = 0;
    bool failed = false;
    boost::fibers::fiber f1( boost::bind( f7, boost::ref( i), boost::ref( failed) ) );
    boost::fibers::fiber f2( boost::bind( interruption_point_join, boost::ref( f1) ) );
    f1.interrupt();
    f2.join();
    BOOST_CHECK_EQUAL( 1, i);
    BOOST_CHECK( failed);
    BOOST_CHECK_EQUAL( 1, i);
}

boost::unit_test::test_suite * init_unit_test_suite( int, char* [])
{
    boost::unit_test::test_suite * test =
        BOOST_TEST_SUITE("Boost.Fiber: fiber test suite");

     test->add( BOOST_TEST_CASE( & test_move) );
     test->add( BOOST_TEST_CASE( & test_id) );
     test->add( BOOST_TEST_CASE( & test_priority) );
     test->add( BOOST_TEST_CASE( & test_detach) );
     test->add( BOOST_TEST_CASE( & test_complete) );
     test->add( BOOST_TEST_CASE( & test_join_in_thread) );
     test->add( BOOST_TEST_CASE( & test_join_and_run) );
     test->add( BOOST_TEST_CASE( & test_join_in_fiber) );
     test->add( BOOST_TEST_CASE( & test_yield) );
     test->add( BOOST_TEST_CASE( & test_fiber_interrupts_at_interruption_point) );
     test->add( BOOST_TEST_CASE( & test_fiber_no_interrupt_if_interrupts_disabled_at_interruption_point) );
     test->add( BOOST_TEST_CASE( & test_fiber_interrupts_at_join) );
     test->add( BOOST_TEST_CASE( & test_replace) );

    return test;
}
