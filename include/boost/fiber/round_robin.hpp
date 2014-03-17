//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_FIBERS_DEFAULT_SCHEDULER_H
#define BOOST_FIBERS_DEFAULT_SCHEDULER_H

#include <algorithm>
#include <deque>
#include <queue>
#include <vector>

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/thread/lock_types.hpp> 

#include <boost/fiber/algorithm.hpp>
#include <boost/fiber/detail/config.hpp>
#include <boost/fiber/detail/fiber_base.hpp>
#include <boost/fiber/detail/fifo.hpp>
#include <boost/fiber/detail/main_fiber.hpp>
#include <boost/fiber/detail/spinlock.hpp>
#include <boost/fiber/detail/worker_fiber.hpp>
#include <boost/fiber/fiber.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

# if defined(BOOST_MSVC)
# pragma warning(push)
# pragma warning(disable:4251 4275)
# endif

namespace boost {
namespace fibers {

class BOOST_FIBERS_DECL round_robin : public algorithm
{
private:
    typedef detail::fifo        wqueue_t;
    typedef detail::fifo        rqueue_t;

    detail::worker_fiber::ptr_t active_fiber_;
    wqueue_t                    wqueue_;
    rqueue_t                    rqueue_;
    detail::main_fiber          mn_;

    detail::worker_fiber::ptr_t pick_next_();

    void resume_( detail::worker_fiber::ptr_t const&);

public:
    round_robin() BOOST_NOEXCEPT;

    ~round_robin() BOOST_NOEXCEPT;

    void spawn( detail::worker_fiber::ptr_t const&);

    void priority( detail::worker_fiber::ptr_t const&, int) BOOST_NOEXCEPT;

    void join( detail::worker_fiber::ptr_t const&);

    detail::worker_fiber::ptr_t active() BOOST_NOEXCEPT
    { return active_fiber_; }

    void run();

    void wait( unique_lock< detail::spinlock > &);
    bool wait_until( clock_type::time_point const&,
                     unique_lock< detail::spinlock > &);

    void yield();

    detail::fiber_base::ptr_t get_main_fiber()
    { return detail::fiber_base::ptr_t( new detail::main_fiber() ); }
};

}}

# if defined(BOOST_MSVC)
# pragma warning(pop)
# endif

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_FIBERS_DEFAULT_SCHEDULER_H
