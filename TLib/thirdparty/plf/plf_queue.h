// Copyright (c) 2023, Matthew Bentley (mattreecebentley@gmail.com) www.plflib.org

// zLib license (https://www.zlib.net/zlib_license.html):
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
// 	claim that you wrote the original software. If you use this software
// 	in a product, an acknowledgement in the product documentation would be
// 	appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// 	misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.


#ifndef PLF_QUEUE_H
#define PLF_QUEUE_H


// Compiler-specific defines:

// Define default cases before possibly redefining:
#define PLF_NOEXCEPT throw()
#define PLF_NOEXCEPT_ALLOCATOR
#define PLF_CONSTEXPR
#define PLF_CONSTFUNC

#define PLF_EXCEPTIONS_SUPPORT

#if ((defined(__clang__) || defined(__GNUC__)) && !defined(__EXCEPTIONS)) || (defined(_MSC_VER) && !defined(_CPPUNWIND))
	#undef PLF_EXCEPTIONS_SUPPORT
	#include <exception> // std::terminate
#endif

#if defined(_MSC_VER) && !defined(__clang__) && !defined(__GNUC__)
	#if _MSC_VER >= 1600
		#define PLF_MOVE_SEMANTICS_SUPPORT
	#endif
	#if _MSC_VER >= 1700
		#define PLF_TYPE_TRAITS_SUPPORT
		#define PLF_ALLOCATOR_TRAITS_SUPPORT
	#endif
	#if _MSC_VER >= 1800
		#define PLF_VARIADICS_SUPPORT // Variadics, in this context, means both variadic templates and variadic macros are supported
		#define PLF_DEFAULT_TEMPLATE_ARGUMENT_SUPPORT
		#define PLF_INITIALIZER_LIST_SUPPORT
	#endif
	#if _MSC_VER >= 1900
		#define PLF_ALIGNMENT_SUPPORT
		#undef PLF_NOEXCEPT
		#undef PLF_NOEXCEPT_ALLOCATOR
		#define PLF_NOEXCEPT noexcept
		#define PLF_NOEXCEPT_ALLOCATOR noexcept(noexcept(allocator_type()))
		#define PLF_IS_ALWAYS_EQUAL_SUPPORT
	#endif

	#if defined(_MSVC_LANG) && (_MSVC_LANG >= 201703L)
		#undef PLF_CONSTEXPR
		#define PLF_CONSTEXPR constexpr
	#endif

	#if defined(_MSVC_LANG) && (_MSVC_LANG >= 202002L) && _MSC_VER >= 1929
		#define PLF_CPP20_SUPPORT
		#undef PLF_CONSTFUNC
		#define PLF_CONSTFUNC constexpr
	#endif
#elif defined(__cplusplus) && __cplusplus >= 201103L // C++11 support, at least
	#if defined(__GNUC__) && defined(__GNUC_MINOR__) && !defined(__clang__) // If compiler is GCC/G++
		#if (__GNUC__ == 4 && __GNUC_MINOR__ >= 3) || __GNUC__ > 4
			#define PLF_MOVE_SEMANTICS_SUPPORT
			#define PLF_VARIADICS_SUPPORT
		#endif
		#if (__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || __GNUC__ > 4
			#define PLF_DEFAULT_TEMPLATE_ARGUMENT_SUPPORT
			#define PLF_INITIALIZER_LIST_SUPPORT
		#endif
		#if (__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4
			#undef PLF_NOEXCEPT
			#undef PLF_NOEXCEPT_ALLOCATOR
			#define PLF_NOEXCEPT noexcept
			#define PLF_NOEXCEPT_ALLOCATOR noexcept(noexcept(allocator_type()))
		#endif
		#if (__GNUC__ == 4 && __GNUC_MINOR__ >= 7) || __GNUC__ > 4
			#define PLF_ALLOCATOR_TRAITS_SUPPORT
		#endif
		#if (__GNUC__ == 4 && __GNUC_MINOR__ >= 8) || __GNUC__ > 4
			#define PLF_ALIGNMENT_SUPPORT
		#endif
		#if __GNUC__ >= 5 // GCC v4.9 and below do not support std::is_trivially_copyable
			#define PLF_TYPE_TRAITS_SUPPORT
		#endif
		#if __GNUC__ > 6
			#define PLF_IS_ALWAYS_EQUAL_SUPPORT
		#endif
	#elif defined(__clang__) && !defined(__GLIBCXX__) && !defined(_LIBCPP_CXX03_LANG) && __clang_major__ >= 3
		#define PLF_DEFAULT_TEMPLATE_ARGUMENT_SUPPORT
		#define PLF_ALLOCATOR_TRAITS_SUPPORT
		#define PLF_TYPE_TRAITS_SUPPORT

		#if __has_feature(cxx_alignas) && __has_feature(cxx_alignof)
			#define PLF_ALIGNMENT_SUPPORT
		#endif
		#if __has_feature(cxx_noexcept)
			#undef PLF_NOEXCEPT
			#undef PLF_NOEXCEPT_ALLOCATOR
			#define PLF_NOEXCEPT noexcept
			#define PLF_NOEXCEPT_ALLOCATOR noexcept(noexcept(allocator_type()))
			#define PLF_IS_ALWAYS_EQUAL_SUPPORT
		#endif
		#if __has_feature(cxx_rvalue_references) && !defined(_LIBCPP_HAS_NO_RVALUE_REFERENCES)
			#define PLF_MOVE_SEMANTICS_SUPPORT
		#endif
		#if __has_feature(cxx_variadic_templates) && !defined(_LIBCPP_HAS_NO_VARIADICS)
			#define PLF_VARIADICS_SUPPORT
		#endif
		#if (__clang_major__ == 3 && __clang_minor__ >= 1) || __clang_major__ > 3
			#define PLF_INITIALIZER_LIST_SUPPORT
		#endif
	#elif defined(__GLIBCXX__) // Using another compiler type with libstdc++ - we are assuming full c++11 compliance for compiler - which may not be true
		#define PLF_DEFAULT_TEMPLATE_ARGUMENT_SUPPORT

		#if __GLIBCXX__ >= 20080606
			#define PLF_MOVE_SEMANTICS_SUPPORT
			#define PLF_VARIADICS_SUPPORT
		#endif
		#if __GLIBCXX__ >= 20090421
			#define PLF_INITIALIZER_LIST_SUPPORT
		#endif
		#if __GLIBCXX__ >= 20120322
			#define PLF_ALLOCATOR_TRAITS_SUPPORT
			#undef PLF_NOEXCEPT
			#undef PLF_NOEXCEPT_ALLOCATOR
			#define PLF_NOEXCEPT noexcept
			#define PLF_NOEXCEPT_ALLOCATOR noexcept(noexcept(allocator_type()))
		#endif
		#if __GLIBCXX__ >= 20130322
			#define PLF_ALIGNMENT_SUPPORT
		#endif
		#if __GLIBCXX__ >= 20150422 // libstdc++ v4.9 and below do not support std::is_trivially_copyable
			#define PLF_TYPE_TRAITS_SUPPORT
		#endif
		#if __GLIBCXX__ >= 20160111
			#define PLF_IS_ALWAYS_EQUAL_SUPPORT
		#endif
	#elif defined(_LIBCPP_CXX03_LANG) || defined(_LIBCPP_HAS_NO_RVALUE_REFERENCES) // Special case for checking C++11 support with libCPP
		#if !defined(_LIBCPP_HAS_NO_VARIADICS)
			#define PLF_VARIADICS_SUPPORT
		#endif
	#else // Assume type traits and initializer support for other compilers and standard library implementations
		#define PLF_DEFAULT_TEMPLATE_ARGUMENT_SUPPORT
		#define PLF_MOVE_SEMANTICS_SUPPORT
		#define PLF_VARIADICS_SUPPORT
		#define PLF_TYPE_TRAITS_SUPPORT
		#define PLF_ALLOCATOR_TRAITS_SUPPORT
		#define PLF_ALIGNMENT_SUPPORT
		#define PLF_INITIALIZER_LIST_SUPPORT
		#undef PLF_NOEXCEPT
		#undef PLF_NOEXCEPT_ALLOCATOR
		#define PLF_NOEXCEPT noexcept
		#define PLF_NOEXCEPT_ALLOCATOR noexcept(noexcept(allocator_type()))
		#define PLF_IS_ALWAYS_EQUAL_SUPPORT
	#endif

	#if __cplusplus >= 201703L && ((defined(__clang__) && ((__clang_major__ == 3 && __clang_minor__ == 9) || __clang_major__ > 3)) || (defined(__GNUC__) && __GNUC__ >= 7) || (!defined(__clang__) && !defined(__GNUC__))) // assume correct C++17 implementation for non-gcc/clang compilers
		#undef PLF_CONSTEXPR
		#define PLF_CONSTEXPR constexpr
	#endif

	#if __cplusplus > 201704L && ((((defined(__clang__) && !defined(__APPLE_CC__) && __clang_major__ >= 14) || (defined(__GNUC__) && (__GNUC__ > 11 || (__GNUC__ == 11 && __GNUC_MINOR__ > 0)))) && ((defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 14) || (defined(__GLIBCXX__) && __GLIBCXX__ >= 201806L))) || (!defined(__clang__) && !defined(__GNUC__)))
		#define PLF_CPP20_SUPPORT
		#undef PLF_CONSTFUNC
		#define PLF_CONSTFUNC constexpr
	#endif
#endif

#if defined(PLF_IS_ALWAYS_EQUAL_SUPPORT) && defined(PLF_MOVE_SEMANTICS_SUPPORT) && defined(PLF_ALLOCATOR_TRAITS_SUPPORT) && (__cplusplus >= 201703L || (defined(_MSVC_LANG) && (_MSVC_LANG >= 201703L)))
	#define PLF_NOEXCEPT_MOVE_ASSIGN(the_allocator) noexcept(std::allocator_traits<the_allocator>::propagate_on_container_move_assignment::value || std::allocator_traits<the_allocator>::is_always_equal::value)
	#define PLF_NOEXCEPT_SWAP(the_allocator) noexcept(std::allocator_traits<the_allocator>::propagate_on_container_swap::value || std::allocator_traits<the_allocator>::is_always_equal::value)
#else
	#define PLF_NOEXCEPT_MOVE_ASSIGN(the_allocator)
	#define PLF_NOEXCEPT_SWAP(the_allocator)
#endif

#undef PLF_IS_ALWAYS_EQUAL_SUPPORT


#ifdef PLF_ALLOCATOR_TRAITS_SUPPORT
	#ifdef PLF_VARIADICS_SUPPORT
		#define PLF_CONSTRUCT(the_allocator, allocator_instance, location, ...) std::allocator_traits<the_allocator>::construct(allocator_instance, location, __VA_ARGS__)
	#else
		#define PLF_CONSTRUCT(the_allocator, allocator_instance, location, data)	std::allocator_traits<the_allocator>::construct(allocator_instance, location, data)
	#endif

	#define PLF_DESTROY(the_allocator, allocator_instance, location)				std::allocator_traits<the_allocator>::destroy(allocator_instance, location)
	#define PLF_ALLOCATE(the_allocator, allocator_instance, size, hint)			std::allocator_traits<the_allocator>::allocate(allocator_instance, size, hint)
	#define PLF_DEALLOCATE(the_allocator, allocator_instance, location, size)	std::allocator_traits<the_allocator>::deallocate(allocator_instance, location, size)
#else
	#ifdef PLF_VARIADICS_SUPPORT
		#define PLF_CONSTRUCT(the_allocator, allocator_instance, location, ...) 	(allocator_instance).construct(location, __VA_ARGS__)
	#else
		#define PLF_CONSTRUCT(the_allocator, allocator_instance, location, data)	(allocator_instance).construct(location, data)
	#endif

	#define PLF_DESTROY(the_allocator, allocator_instance, location)				(allocator_instance).destroy(location)
	#define PLF_ALLOCATE(the_allocator, allocator_instance, size, hint)			(allocator_instance).allocate(size, hint)
	#define PLF_DEALLOCATE(the_allocator, allocator_instance, location, size)	(allocator_instance).deallocate(location, size)
#endif




#include <cstring> // memset, memcpy
#include <cassert> // assert
#include <limits>  // std::numeric_limits
#include <memory> // std::uninitialized_copy, std::allocator
#include <stdexcept> // std::length_error

#ifdef PLF_MOVE_SEMANTICS_SUPPORT
	#include <utility> // std::move
#endif

#ifdef PLF_TYPE_TRAITS_SUPPORT
	#include <cstddef> // offsetof, used in blank()
	#include <type_traits> // std::is_trivially_destructible
#endif




namespace plf
{

struct queue_limits // for use in block_capacity setting/getting functions and constructors
{
	size_t min, max;
	queue_limits(const size_t minimum, const size_t maximum) PLF_NOEXCEPT : min(minimum), max(maximum) {}
};


enum queue_priority { speed = 1, memory = 4};


template <class element_type, plf::queue_priority priority = plf::memory, class allocator_type = std::allocator<element_type> > class queue : private allocator_type // Empty base class optimisation - inheriting allocator functions
{
public:
	// Standard container typedefs:
	typedef element_type																			value_type;

	#ifdef PLF_ALLOCATOR_TRAITS_SUPPORT
		typedef typename std::allocator_traits<allocator_type>::size_type			size_type;
		typedef element_type &																	reference;
		typedef const element_type &															const_reference;
		typedef typename std::allocator_traits<allocator_type>::pointer 			pointer;
		typedef typename std::allocator_traits<allocator_type>::const_pointer	const_pointer;
	#else
		typedef typename allocator_type::size_type			size_type;
		typedef typename allocator_type::reference			reference;
		typedef typename allocator_type::const_reference	const_reference;
		typedef typename allocator_type::pointer				pointer;
		typedef typename allocator_type::const_pointer		const_pointer;
	#endif

private:
	struct group; // Forward declaration for typedefs below

	#ifdef PLF_ALLOCATOR_TRAITS_SUPPORT
		typedef typename std::allocator_traits<allocator_type>::template rebind_alloc<group> group_allocator_type;
		typedef typename std::allocator_traits<group_allocator_type>::pointer					group_pointer_type;
		typedef typename std::allocator_traits<allocator_type>::pointer 							element_pointer_type;
	#else
		typedef typename allocator_type::template rebind<group>::other	group_allocator_type;
		typedef typename group_allocator_type::pointer						group_pointer_type;
		typedef typename allocator_type::pointer								element_pointer_type;
	#endif


	struct group : private allocator_type
	{
		const element_pointer_type		elements;
		group_pointer_type				next_group, previous_group;
		const element_pointer_type		end; // One-past the back element


		#ifdef PLF_VARIADICS_SUPPORT
			group(const size_type elements_per_group, group_pointer_type const previous = NULL):
				elements(PLF_ALLOCATE(allocator_type, *this, elements_per_group, (previous == NULL) ? 0 : previous->elements)),
				next_group(NULL),
				previous_group(previous),
				end(elements + elements_per_group)
			{}


		#else
			// This is a hack around the fact that allocator_type::construct only supports copy construction in C++03 and copy elision does not occur on the vast majority of compilers in this circumstance. And to avoid running out of memory (and performance loss) from allocating the same block twice, we're allocating in this constructor and moving data in the copy constructor.
			group(const size_type elements_per_group, group_pointer_type const previous = NULL) PLF_NOEXCEPT:
				elements(NULL),
				next_group(reinterpret_cast<group_pointer_type>(elements_per_group)),
				previous_group(previous),
				end(NULL)
			{}


			// Not a real copy constructor ie. actually a move constructor. Only used for allocator.construct in C++03 for reasons stated above:
			group(const group &source):
				allocator_type(source),
				elements(PLF_ALLOCATE(allocator_type, *this, reinterpret_cast<size_type>(source.next_group), (source.previous_group == NULL) ? 0 : source.previous_group->elements)),
				next_group(NULL),
				previous_group(source.previous_group),
				end(elements + reinterpret_cast<size_type>(source.next_group))
			{}
		#endif



		~group() PLF_NOEXCEPT
		{
			// Null check not necessary (for empty group and copied group as above) as deallocate will do it's own null check.
			PLF_DEALLOCATE(allocator_type, *this, elements, static_cast<size_type>(end - elements));
		}
	};


	group_pointer_type		current_group, first_group; // current group is location of top pointer, first_group is 'front' group, saves performance for ~queue etc
	element_pointer_type		top_element, start_element, end_element; // start_element/end_element cache current_group->end/elements for better performance
	size_type					total_size, total_capacity, min_block_capacity;
	struct ebco_pair : group_allocator_type // Packaging the group allocator with the least-used member variable, for empty-base-class optimization
	{
		size_type max_block_capacity;
		ebco_pair(const size_type max_elements, const allocator_type &alloc) PLF_NOEXCEPT:
			group_allocator_type(alloc),
			max_block_capacity(max_elements)
		{};
	} group_allocator_pair;



	void check_capacities_conformance(const size_type min, const size_type max) const
	{
  		if (min < 2 || min > max || max > (std::numeric_limits<size_type>::max() / 2))
		{
			#ifdef PLF_EXCEPTIONS_SUPPORT
				throw std::length_error("Supplied memory block capacities outside of allowable ranges");
			#else
				std::terminate();
			#endif
		}
	}


	#define PLF_MIN_BLOCK_CAPACITY ((sizeof(element_type) * 8 > (sizeof(*this) + sizeof(group)) * 2) ? 8 : (((sizeof(*this) + sizeof(group)) * 2) / sizeof(element_type)) + 1) / priority
	#define PLF_MAX_BLOCK_CAPACITY ((sizeof(element_type) > 128) ? 768 : 12288 / sizeof(element_type)) / priority

public:

	// Default constructor:
	PLF_CONSTFUNC queue() PLF_NOEXCEPT_ALLOCATOR:
		current_group(NULL),
		first_group(NULL),
		top_element(NULL),
		start_element(NULL),
		end_element(NULL),
		total_size(0),
		total_capacity(0),
		min_block_capacity(PLF_MIN_BLOCK_CAPACITY),
		group_allocator_pair(PLF_MAX_BLOCK_CAPACITY, *this)
	{}


	// Allocator-extended constructor:
	explicit queue(const allocator_type &alloc):
		allocator_type(alloc),
		current_group(NULL),
		first_group(NULL),
		top_element(NULL),
		start_element(NULL),
		end_element(NULL),
		total_size(0),
		total_capacity(0),
		min_block_capacity(PLF_MIN_BLOCK_CAPACITY),
		group_allocator_pair(PLF_MAX_BLOCK_CAPACITY, alloc)
	{}



	// Constructor with minimum & maximum group size parameters:
	queue(const size_type min, const size_type max = PLF_MAX_BLOCK_CAPACITY):
		current_group(NULL),
		first_group(NULL),
		top_element(NULL),
		start_element(NULL),
		end_element(NULL),
		total_size(0),
		total_capacity(0),
		min_block_capacity(min),
		group_allocator_pair(max, *this)
	{
		check_capacities_conformance(min, max);
	}



	// Allocator-extended constructor with minimum & maximum group size parameters:
	queue(const size_type min, const size_type max, const allocator_type &alloc):
		allocator_type(alloc),
		current_group(NULL),
		first_group(NULL),
		top_element(NULL),
		start_element(NULL),
		end_element(NULL),
		total_size(0),
		total_capacity(0),
		min_block_capacity(min),
		group_allocator_pair(max, alloc)
	{
		check_capacities_conformance(min, max);
	}



private:

	void allocate_new_group(const size_type capacity, group_pointer_type const previous_group)
	{
		previous_group->next_group = PLF_ALLOCATE(group_allocator_type, group_allocator_pair, 1, previous_group);

		#ifdef PLF_EXCEPTIONS_SUPPORT
			try
			{
				#ifdef PLF_VARIADICS_SUPPORT
					PLF_CONSTRUCT(group_allocator_type, group_allocator_pair, previous_group->next_group, capacity, previous_group);
				#else
					PLF_CONSTRUCT(group_allocator_type, group_allocator_pair, previous_group->next_group, group(capacity, previous_group));
				#endif
			}
			catch (...)
			{
				PLF_DEALLOCATE(group_allocator_type, group_allocator_pair, previous_group->next_group, 1);
				throw;
			}
		#else
			#ifdef PLF_VARIADICS_SUPPORT
				PLF_CONSTRUCT(group_allocator_type, group_allocator_pair, previous_group->next_group, capacity, previous_group);
			#else
				PLF_CONSTRUCT(group_allocator_type, group_allocator_pair, previous_group->next_group, group(capacity, previous_group));
			#endif
		#endif

		total_capacity += capacity;
	}



	void deallocate_group(group_pointer_type const the_group) PLF_NOEXCEPT
	{
		PLF_DESTROY(group_allocator_type, group_allocator_pair, the_group);
		PLF_DEALLOCATE(group_allocator_type, group_allocator_pair, the_group, 1);
	}



	void initialize()
	{
		first_group = current_group = PLF_ALLOCATE(group_allocator_type, group_allocator_pair, 1, 0);

		#ifdef PLF_EXCEPTIONS_SUPPORT
			try
			{
				#ifdef PLF_VARIADICS_SUPPORT
					PLF_CONSTRUCT(group_allocator_type, group_allocator_pair, first_group, min_block_capacity);
				#else
					PLF_CONSTRUCT(group_allocator_type, group_allocator_pair, first_group, group(min_block_capacity));
				#endif
			}
			catch (...)
			{
				PLF_DEALLOCATE(group_allocator_type, group_allocator_pair, first_group, 1);
				first_group = current_group = NULL;
				throw;
			}
		#else
			#ifdef PLF_VARIADICS_SUPPORT
				PLF_CONSTRUCT(group_allocator_type, group_allocator_pair, first_group, min_block_capacity);
			#else
				PLF_CONSTRUCT(group_allocator_type, group_allocator_pair, first_group, group(min_block_capacity));
			#endif
		#endif

		start_element = top_element = first_group->elements;
		end_element = first_group->end;
		total_capacity = min_block_capacity;
	}



	void progress_to_next_group() // used by push/emplace
	{
		if (current_group->next_group == NULL) // no reserved groups or groups left over from previous pops, allocate new group
		{
			// Logic: if total_size > current group capacity * 1.25 or total_size < current group capacity * .75, make new group capacity == total_size, otherwise make it same as current group capacity. The new size is then truncated if necessary to fit within the user-specified min/max block sizes. This means we are more likely to end up with blocks of equal capacity when the total size is not increasing or lowering significantly over time - which in turn means previous blocks can get reused when they become empty.
			const size_type current_group_capacity = static_cast<size_type>(current_group->end - current_group->elements);
			const size_type divided_size = total_size / priority;
			const size_type new_group_capacity = ((divided_size < static_cast<size_type>(current_group_capacity * 2)) & (divided_size > static_cast<size_type>(current_group_capacity * .5))) ? current_group_capacity :
															(divided_size < min_block_capacity) ? min_block_capacity :
															(divided_size > group_allocator_pair.max_block_capacity) ? group_allocator_pair.max_block_capacity : divided_size;
			allocate_new_group(new_group_capacity, current_group);
		}

		current_group = current_group->next_group;
		top_element = current_group->elements;
		end_element = current_group->end;
	}



	void copy_from_source(const queue &source) // Note: this function is only called on an empty un-initialize()'d queue
	{
		assert(&source != this);

		if (source.total_size == 0)
		{
			return;
		}

		group_pointer_type current_copy_group = source.first_group;
		const group_pointer_type end_copy_group = source.current_group;

		if (source.total_size <= group_allocator_pair.max_block_capacity) // most common case
		{
			const size_type original_min_block_capacity = min_block_capacity;
			min_block_capacity = source.total_size;
			initialize();
			min_block_capacity = original_min_block_capacity;

			element_pointer_type start_pointer = source.start_element;

			// Copy groups to this queue:
			while (current_copy_group != end_copy_group)
			{
				std::uninitialized_copy(start_pointer, current_copy_group->end, top_element);
				top_element += current_copy_group->end - start_pointer;
				current_copy_group = current_copy_group->next_group;
				start_pointer = current_copy_group->elements;
			}

			// Handle special case of last group:
			std::uninitialized_copy(start_pointer, source.top_element + 1, top_element);
			top_element += source.top_element - start_pointer; // This should make top_element == the last "pushed" element, rather than the one past it
			end_element = top_element + 1; // Since we have created a single group where capacity == size, this is correct
			total_size = source.total_size;
		}
		else // uncommon edge case, so not optimising:
		{
			min_block_capacity = group_allocator_pair.max_block_capacity;
			element_pointer_type start_pointer = source.start_element;

			while (current_copy_group != end_copy_group)
			{
				for (element_pointer_type element_to_copy = start_pointer; element_to_copy != current_copy_group->end; ++element_to_copy)
				{
					push(*element_to_copy);
				}

				current_copy_group = current_copy_group->next_group;
				start_pointer = current_copy_group->elements;
			}

			// Handle special case of last group:
			for (element_pointer_type element_to_copy = start_pointer; element_to_copy != source.top_element + 1; ++element_to_copy)
			{
				push(*element_to_copy);
			}

			min_block_capacity = source.min_block_capacity;
		}
	}



	void destroy_all_data() PLF_NOEXCEPT
	{
		#ifdef PLF_TYPE_TRAITS_SUPPORT
			if PLF_CONSTEXPR(!std::is_trivially_destructible<element_type>::value) // Avoid iteration for trivially-destructible types eg. POD, structs, classes with empty destructor.
		#endif // If compiler doesn't support traits, iterate regardless - trivial destructors will not be called, hopefully compiler will optimise this loop out for POD types
		{
			if (total_size != 0)
			{
				while (first_group != current_group)
				{
					const element_pointer_type past_end = first_group->end;

					for (element_pointer_type element_pointer = start_element; element_pointer != past_end; ++element_pointer)
					{
						PLF_DESTROY(allocator_type, *this, element_pointer);
					}

					const group_pointer_type next_group = first_group->next_group;
					deallocate_group(first_group);
					first_group = next_group;
					start_element = next_group->elements;
				}

				// Special case for current group:
				const element_pointer_type past_end = top_element + 1;

				for (element_pointer_type element_pointer = start_element; element_pointer != past_end; ++element_pointer)
				{
					PLF_DESTROY(allocator_type, *this, element_pointer);
				}

				first_group = first_group->next_group; // To further process reserved groups in the following loop
				deallocate_group(current_group);
			}
		}

		total_size = 0;

		while (first_group != NULL)
		{
			current_group = first_group;
			first_group = first_group->next_group;
			deallocate_group(current_group);
		}
	}



	void blank() PLF_NOEXCEPT
	{
		#ifdef PLF_TYPE_TRAITS_SUPPORT
			if PLF_CONSTEXPR (std::is_standard_layout<queue>::value && std::allocator_traits<allocator_type>::is_always_equal::value && std::is_trivial<group_pointer_type>::value && std::is_trivial<element_pointer_type>::value) // if all pointer types are trivial, we can just nuke it from orbit with memset (NULL is always 0 in C++):
			{
				std::memset(static_cast<void *>(this), 0, offsetof(queue, min_block_capacity));
			}
			else
		#endif
		{
			current_group = NULL;
			first_group = NULL;
			top_element = NULL;
			start_element = NULL;
			end_element = NULL;
			total_size = 0;
			total_capacity = 0;
		}
	}



public:

	// Copy constructor:
	queue(const queue &source):
		#if defined(__cplusplus) && __cplusplus >= 201103L
			allocator_type(std::allocator_traits<allocator_type>::select_on_container_copy_construction(source)),
		#else
			allocator_type(source),
		#endif
		current_group(NULL),
		first_group(NULL),
		top_element(NULL),
		start_element(NULL),
		end_element(NULL),
		total_size(0),
		total_capacity(0),
		min_block_capacity(source.min_block_capacity),
		group_allocator_pair(source.group_allocator_pair.max_block_capacity, *this)
	{
		copy_from_source(source);
	}



	// Allocator-extended copy constructor:
	queue(const queue &source, const allocator_type &alloc):
		allocator_type(alloc),
		current_group(NULL),
		first_group(NULL),
		top_element(NULL),
		start_element(NULL),
		end_element(NULL),
		total_size(0),
		total_capacity(0),
		min_block_capacity(source.min_block_capacity),
		group_allocator_pair(source.group_allocator_pair.max_block_capacity, alloc)
	{
		copy_from_source(source);
	}



	#ifdef PLF_MOVE_SEMANTICS_SUPPORT
		// move constructor
		queue(queue &&source) PLF_NOEXCEPT:
			allocator_type(std::move(static_cast<allocator_type &>(source))),
			current_group(std::move(source.current_group)),
			first_group(std::move(source.first_group)),
			top_element(std::move(source.top_element)),
			start_element(std::move(source.start_element)),
			end_element(std::move(source.end_element)),
			total_size(source.total_size),
			total_capacity(source.total_capacity),
			min_block_capacity(source.min_block_capacity),
			group_allocator_pair(source.group_allocator_pair.max_block_capacity, source)
		{
			source.blank();
		}


		// allocator-extended move constructor
		queue(queue &&source, const allocator_type &alloc):
			allocator_type(alloc),
			current_group(std::move(source.current_group)),
			first_group(std::move(source.first_group)),
			top_element(std::move(source.top_element)),
			start_element(std::move(source.start_element)),
			end_element(std::move(source.end_element)),
			total_size(source.total_size),
			total_capacity(source.total_capacity),
			min_block_capacity(source.min_block_capacity),
			group_allocator_pair(source.group_allocator_pair.max_block_capacity, alloc)
		{
			source.blank();
		}
	#endif



	~queue() PLF_NOEXCEPT
	{
		destroy_all_data();
	}



	void push(const element_type &element)
	{
		if (top_element == NULL)
		{
			initialize();
		}
		else if (++top_element == end_element) // ie. out of capacity for current element memory block
		{
			progress_to_next_group();
		}

		// Create element:
		#ifdef PLF_TYPE_TRAITS_SUPPORT
			if PLF_CONSTEXPR (std::is_nothrow_copy_constructible<element_type>::value)
			{
				PLF_CONSTRUCT(allocator_type, *this, top_element, element);
			}
			else
		#endif
		{
			#ifdef PLF_EXCEPTIONS_SUPPORT
				try
				{
					PLF_CONSTRUCT(allocator_type, *this, top_element, element);
				}
				catch (...)
				{
					if (top_element == start_element && total_size != 0) // for post-initialize push
					{
						current_group = current_group->previous_group;
						start_element = current_group->elements;
						top_element = current_group->end - 1;
					}
					else
					{
						--top_element;
					}

					throw;
				}
			#else
				PLF_CONSTRUCT(allocator_type, *this, top_element, element);
			#endif
		}

		++total_size;
	}



	#ifdef PLF_MOVE_SEMANTICS_SUPPORT
		// Note: the reason for code duplication from non-move push, as opposed to using std::forward for both, was because most compilers didn't actually create as-optimal code in that strategy. Also C++03 compatibility.
		void push(element_type &&element)
		{
			if (top_element == NULL)
			{
				initialize();
			}
			else if (++top_element == end_element)
			{
				progress_to_next_group();
			}


			#ifdef PLF_TYPE_TRAITS_SUPPORT
				if PLF_CONSTEXPR (std::is_nothrow_move_constructible<element_type>::value)
				{
					PLF_CONSTRUCT(allocator_type, *this, top_element, std::move(element));
				}
				else
			#endif
			{
				#ifdef PLF_EXCEPTIONS_SUPPORT
					try
					{
						PLF_CONSTRUCT(allocator_type, *this, top_element, std::move(element));
					}
					catch (...)
					{
						if (top_element == start_element && total_size != 0)
						{
							current_group = current_group->previous_group;
							start_element = current_group->elements;
							top_element = current_group->end - 1;
						}
						else
						{
							--top_element;
						}

						throw;
					}
				#else
					PLF_CONSTRUCT(allocator_type, *this, top_element, std::move(element));
				#endif
			}

			++total_size;
		}
	#endif




	#ifdef PLF_VARIADICS_SUPPORT
		template<typename... arguments>
		void emplace(arguments &&... parameters)
		{
			if (top_element == NULL)
			{
				initialize();
			}
			else if (++top_element == end_element)
			{
				progress_to_next_group();
			}


			#ifdef PLF_TYPE_TRAITS_SUPPORT
				if PLF_CONSTEXPR (std::is_nothrow_move_constructible<element_type>::value)
				{
					PLF_CONSTRUCT(allocator_type, *this, top_element, std::forward<arguments>(parameters)...);
				}
				else
			#endif
			{
				#ifdef PLF_EXCEPTIONS_SUPPORT
					try
					{
						PLF_CONSTRUCT(allocator_type, *this, top_element, std::forward<arguments>(parameters)...);
					}
					catch (...)
					{
						if (top_element == start_element && total_size != 0)
						{
							current_group = current_group->previous_group;
							start_element = current_group->elements;
							top_element = current_group->end - 1;
						}
						else
						{
							--top_element;
						}

						throw;
					}
				#else
					PLF_CONSTRUCT(allocator_type, *this, top_element, std::forward<arguments>(parameters)...);
				#endif
			}

			++total_size;
		}
	#endif



	reference front() const // Exception may occur if queue is empty in release mode
	{
		assert(total_size != 0);
		return *start_element;
	}



	reference back() const // Exception may occur if queue is empty in release mode
	{
		assert(total_size != 0);
		return *top_element;
	}



	void pop() // Exception may occur if queue is empty
	{
		assert(total_size != 0);

		#ifdef PLF_TYPE_TRAITS_SUPPORT
			if PLF_CONSTEXPR (!std::is_trivially_destructible<element_type>::value)
		#endif
		{
			PLF_DESTROY(allocator_type, *this, start_element);
		}

		if (--total_size == 0)
		{
			start_element = first_group->elements;
			end_element = first_group->end;
			top_element = start_element - 1;
		}
		else if (++start_element == first_group->end)
		{ // ie. is start element, but not first group in queue
			const group_pointer_type next_group = first_group->next_group;

			if (current_group->next_group == NULL && ((first_group->end - first_group->elements) == (current_group->end - current_group->elements)))
			{
				current_group->next_group = first_group;
				first_group->next_group = NULL;
			}
			else
			{
				total_capacity -= static_cast<size_type>(first_group->end - first_group->elements);
				deallocate_group(first_group);
			}

			first_group = next_group;
			start_element = next_group->elements;
		}
	}



	queue & operator = (const queue &source)
	{
		assert(&source != this);

		destroy_all_data();
		queue temp(source);

		#ifdef PLF_MOVE_SEMANTICS_SUPPORT
			*this = std::move(temp); // Avoid generating 2nd temporary
		#else
			swap(temp);
		#endif

		return *this;
	}



	#ifdef PLF_MOVE_SEMANTICS_SUPPORT
		// Move assignment
		queue & operator = (queue &&source) PLF_NOEXCEPT_MOVE_ASSIGN(allocator_type)
		{
			assert (&source != this);

			destroy_all_data();

			#ifdef PLF_ALLOCATOR_TRAITS_SUPPORT
				if (std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value || std::allocator_traits<allocator_type>::is_always_equal::value || static_cast<allocator_type &>(*this) == static_cast<allocator_type &>(source))
			#else
				if (static_cast<allocator_type &>(*this) == static_cast<allocator_type &>(source))
			#endif
			{
				#if defined(PLF_TYPE_TRAITS_SUPPORT) && defined(PLF_ALLOCATOR_TRAITS_SUPPORT)
					if PLF_CONSTEXPR ((std::is_trivially_copyable<allocator_type>::value || std::allocator_traits<allocator_type>::is_always_equal::value) &&
						std::is_trivial<group_pointer_type>::value && std::is_trivial<element_pointer_type>::value)
					{
						std::memcpy(static_cast<void *>(this), &source, sizeof(queue));
					}
					else
				#endif
				{
					current_group = std::move(source.current_group);
					first_group = std::move(source.first_group);
					top_element = std::move(source.top_element);
					start_element = std::move(source.start_element);
					end_element = std::move(source.end_element);
					total_size = source.total_size;
					total_capacity = source.total_capacity;
					min_block_capacity = source.min_block_capacity;
					group_allocator_pair.max_block_capacity = source.group_allocator_pair.max_block_capacity;

					#ifdef PLF_ALLOCATOR_TRAITS_SUPPORT
						if PLF_CONSTEXPR(std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value)
					#endif
					{
						static_cast<allocator_type &>(*this) = std::move(static_cast<allocator_type &>(source));
						// Reconstruct rebinds:
						static_cast<group_allocator_type &>(group_allocator_pair) = group_allocator_type(*this);
					}
				}
			}
			else // Allocator isn't movable so move elements from source and deallocate the source's blocks:
			{
				queue temp(source);
				swap(temp);
			}

			source.blank();
			return *this;
		}
	#endif



	#ifdef PLF_CPP20_SUPPORT
		[[nodiscard]]
	#endif
	bool empty() const PLF_NOEXCEPT
	{
		return total_size == 0;
	}



	size_type size() const PLF_NOEXCEPT
	{
		return total_size;
	}



	size_type max_size() const PLF_NOEXCEPT
	{
		#ifdef PLF_ALLOCATOR_TRAITS_SUPPORT
			return std::allocator_traits<allocator_type>::max_size(*this);
		#else
			return allocator_type::max_size();
		#endif
	}



	size_type capacity() const PLF_NOEXCEPT
	{
		return total_capacity;
	}



	size_type memory() const PLF_NOEXCEPT
	{
		size_type memory_use = sizeof(*this) + (sizeof(value_type) * total_capacity);
		group_pointer_type temp_group = first_group;

		while (temp_group != NULL)
		{
			memory_use += sizeof(group);
			temp_group = temp_group->next_group;
		}

		return memory_use;
	}



private:

	#ifdef PLF_MOVE_SEMANTICS_SUPPORT
		void move_from_source(queue &source) // This function is a mirror copy of copy_from_source, with move instead of copy
		{
			assert(&source != this);

			if (source.total_size == 0)
			{
				return;
			}

			group_pointer_type current_copy_group = source.first_group;
			const group_pointer_type end_copy_group = source.current_group;

			if (source.total_size <= group_allocator_pair.max_block_capacity) // most common case
			{
				const size_type original_min_block_capacity = min_block_capacity;
				min_block_capacity = source.total_size;
				initialize();
				min_block_capacity = original_min_block_capacity;

				element_pointer_type start_pointer = source.start_element;

				// Copy groups to this queue:
				while (current_copy_group != end_copy_group)
				{
					std::uninitialized_copy(std::make_move_iterator(start_pointer), std::make_move_iterator(current_copy_group->end), top_element);
					top_element += current_copy_group->end - start_pointer;
					current_copy_group = current_copy_group->next_group;
					start_pointer = current_copy_group->elements;
				}

				// Handle special case of last group:
				std::uninitialized_copy(std::make_move_iterator(start_pointer), std::make_move_iterator(source.top_element + 1), top_element);
				top_element += source.top_element - start_pointer; // This should make top_element == the last "pushed" element, rather than the one past it
				end_element = top_element + 1; // Since we have created a single group where capacity == size, this is correct
				total_size = source.total_size;
			}
			else // uncommon edge case, so not optimising:
			{
				min_block_capacity = group_allocator_pair.max_block_capacity;
				element_pointer_type start_pointer = source.start_element;

				while (current_copy_group != end_copy_group)
				{
					for (element_pointer_type element_to_copy = start_pointer; element_to_copy != current_copy_group->end; ++element_to_copy)
					{
						push(std::move(*element_to_copy));
					}

					current_copy_group = current_copy_group->next_group;
					start_pointer = current_copy_group->elements;
				}

				// Handle special case of last group:
				for (element_pointer_type element_to_copy = start_pointer; element_to_copy != source.top_element + 1; ++element_to_copy)
				{
					push(std::move(*element_to_copy));
				}

				min_block_capacity = source.min_block_capacity;
			}
		}
	#endif



	void consolidate()
	{
		#ifdef PLF_MOVE_SEMANTICS_SUPPORT
			queue temp((min_block_capacity > total_size) ? min_block_capacity : ((total_size > group_allocator_pair.max_block_capacity) ? group_allocator_pair.max_block_capacity : total_size), group_allocator_pair.max_block_capacity); // Make first allocated block as large as size(), where possible

			#ifdef PLF_TYPE_TRAITS_SUPPORT
				if PLF_CONSTEXPR (std::is_move_assignable<element_type>::value && std::is_move_constructible<element_type>::value)
				{
					temp.move_from_source(*this);
				}
				else
			#endif
			{
				temp.copy_from_source(*this);
			}

			temp.min_block_capacity = min_block_capacity; // reset to correct value for future clear() or erasures
			*this = std::move(temp);
		#else
			queue temp(*this);
			swap(temp);
		#endif
	}



public:


	void reshape(const size_type min, const size_type max)
	{
		check_capacities_conformance(min, max);

		min_block_capacity = min;
		group_allocator_pair.max_block_capacity = max;

		// Need to check all group sizes, because append might append smaller blocks to the end of a larger block:
		for (group_pointer_type current = first_group; current != NULL; current = current->next_group)
		{
			if (static_cast<size_type>(current->end - current->elements) < min || static_cast<size_type>(current->end - current->elements) > max)
			{
				#ifdef PLF_TYPE_TRAITS_SUPPORT // If type is non-copyable/movable, cannot be consolidated, throw exception:
					if PLF_CONSTEXPR (!((std::is_copy_constructible<element_type>::value && std::is_copy_assignable<element_type>::value) || (std::is_move_constructible<element_type>::value && std::is_move_assignable<element_type>::value)))
					{
						#ifdef PLF_EXCEPTIONS_SUPPORT
							throw;
						#else
							std::terminate();
						#endif
					}
					else
				#endif
				{
					consolidate();
				}

				return;
			}
		}
	}



	void clear() PLF_NOEXCEPT
	{
		destroy_all_data();
		blank();
	}



	bool operator == (const queue &rh) const PLF_NOEXCEPT
	{
		assert (this != &rh);

		if (total_size != rh.total_size)
		{
			return false;
		}

		if (total_size == 0)
		{
			return true;
		}

		group_pointer_type this_group = first_group, rh_group = rh.first_group;
		element_pointer_type this_pointer = start_element, rh_pointer = rh.start_element;

		while(true)
		{
			if (*this_pointer != *rh_pointer)
			{
				return false;
			}

			if (this_pointer == top_element)
			{
				break;
			}

			if (++this_pointer == this_group->end)
			{
				this_group = this_group->next_group;
				this_pointer = this_group->elements;
			}

			if (++rh_pointer == rh_group->end)
			{
				rh_group = rh_group->next_group;
				rh_pointer = rh_group->elements;
			}
		}

		return true;
	}



	bool operator != (const queue &rh) const PLF_NOEXCEPT
	{
		return !(*this == rh);
	}



	// Remove trailing groups (as may be created by reserve or pop)
	void trim() PLF_NOEXCEPT
	{
		if (current_group == NULL) // ie. queue is empty
		{
			return;
		}

		group_pointer_type temp_group = current_group->next_group;
		current_group->next_group = NULL; // Set to NULL regardless of whether it is already NULL (avoids branching). Cuts off rest of groups from this group.

		while (temp_group != NULL)
		{
			const group_pointer_type next_group = temp_group->next_group;
			total_capacity -= static_cast<size_type>(temp_group->end - temp_group->elements);
			deallocate_group(temp_group);
			temp_group = next_group;
		}
	}



	void shrink_to_fit()
	{
		if (first_group == NULL || total_size == capacity())
		{
			return;
		}
		else if (total_size == 0) // Edge case
		{
			clear();
			return;
		}

		consolidate();
	}



	void reserve(size_type reserve_amount)
	{
		if (reserve_amount == 0 || reserve_amount <= total_capacity)
		{
			return;
		}

		reserve_amount -= total_capacity;

		if (reserve_amount < min_block_capacity)
		{
			reserve_amount = min_block_capacity;
		}
		else if (reserve_amount > max_size())
		{
			reserve_amount = max_size();
		}


		size_type number_of_max_capacity_groups = reserve_amount / group_allocator_pair.max_block_capacity,
					remainder = reserve_amount - (number_of_max_capacity_groups * group_allocator_pair.max_block_capacity);

		if (remainder < min_block_capacity)
		{
			remainder = min_block_capacity;
		}

		if (first_group == NULL) // ie. uninitialized queue
		{
			const size_type original_min_elements = min_block_capacity;

			if (remainder != 0)
			{
				min_block_capacity = remainder;
				remainder = 0;
			}
			else
			{
				min_block_capacity = group_allocator_pair.max_block_capacity;
				--number_of_max_capacity_groups;
			}

			initialize();
			--top_element;
			min_block_capacity = original_min_elements;
		}


		group_pointer_type temp_group = current_group;

		// navigate to end of all current reserved groups (if they exist):
		while (temp_group->next_group != NULL)
		{
			temp_group = temp_group->next_group;
		}


		if (remainder != 0)
		{
			allocate_new_group(remainder, temp_group);
			temp_group = temp_group->next_group;
		}


		while(number_of_max_capacity_groups != 0)
		{
			allocate_new_group(group_allocator_pair.max_block_capacity, temp_group);
			temp_group = temp_group->next_group;
			--number_of_max_capacity_groups;
		}
	}



	allocator_type get_allocator() const PLF_NOEXCEPT
	{
		return allocator_type();
	}



	void swap(queue &source) PLF_NOEXCEPT_SWAP(allocator_type)
	{
		#ifdef PLF_TYPE_TRAITS_SUPPORT
			if PLF_CONSTEXPR (std::allocator_traits<allocator_type>::is_always_equal::value && std::is_trivial<group_pointer_type>::value && std::is_trivial<element_pointer_type>::value) // if all pointer types are trivial we can just copy using memcpy - avoids constructors/destructors etc and is faster
			{
				char temp[sizeof(queue)];
				std::memcpy(&temp, static_cast<void *>(this), sizeof(queue));
				std::memcpy(static_cast<void *>(this), static_cast<void *>(&source), sizeof(queue));
				std::memcpy(static_cast<void *>(&source), &temp, sizeof(queue));
			}
			#ifdef PLF_MOVE_SEMANTICS_SUPPORT // If pointer types are not trivial, moving them is probably going to be more efficient than copying them below
				else if PLF_CONSTEXPR (std::is_move_assignable<group_pointer_type>::value && std::is_move_assignable<element_pointer_type>::value && std::is_move_constructible<group_pointer_type>::value && std::is_move_constructible<element_pointer_type>::value)
				{
					queue temp(std::move(source));
					source = std::move(*this);
					*this = std::move(temp);
				}
			#endif
			else
		#endif
		{
			const group_pointer_type	swap_current_group = current_group, swap_first_group = first_group;
			const element_pointer_type	swap_top_element = top_element, swap_start_element = start_element, swap_end_element = end_element;
			const size_type				swap_total_size = total_size, swap_total_capacity = total_capacity, swap_min_block_capacity = min_block_capacity, swap_max_block_capacity = group_allocator_pair.max_block_capacity;

			current_group = source.current_group;
			first_group = source.first_group;
			top_element = source.top_element;
			start_element = source.start_element;
			end_element = source.end_element;
			total_size = source.total_size;
			total_capacity = source.total_capacity;
			min_block_capacity = source.min_block_capacity;
			group_allocator_pair.max_block_capacity = source.group_allocator_pair.max_block_capacity;

			source.current_group = swap_current_group;
			source.first_group = swap_first_group;
			source.top_element = swap_top_element;
			source.start_element = swap_start_element;
			source.end_element = swap_end_element;
			source.total_size = swap_total_size;
			source.total_capacity = swap_total_capacity;
			source.min_block_capacity = swap_min_block_capacity;
			source.group_allocator_pair.max_block_capacity = swap_max_block_capacity;

			#ifdef PLF_ALLOCATOR_TRAITS_SUPPORT
				if PLF_CONSTEXPR (std::allocator_traits<allocator_type>::propagate_on_container_swap::value && !std::allocator_traits<allocator_type>::is_always_equal::value)
			#endif
			{
				#ifdef PLF_MOVE_SEMANTICS_SUPPORT
					allocator_type swap_allocator = std::move(static_cast<allocator_type &>(source));
					static_cast<allocator_type &>(source) = std::move(static_cast<allocator_type &>(*this));
					static_cast<allocator_type &>(*this) = std::move(swap_allocator);
				#else
					allocator_type swap_allocator = static_cast<allocator_type &>(source);
					static_cast<allocator_type &>(source) = static_cast<allocator_type &>(*this);
					static_cast<allocator_type &>(*this) = swap_allocator;
				#endif

				// Reconstruct rebinds for swapped allocators:
				static_cast<group_allocator_type &>(group_allocator_pair) = group_allocator_type(*this);
			} // else: undefined behaviour, as per standard
		}
	}


}; // queue

} // plf namespace



namespace std
{

template <class element_type, plf::queue_priority q_priority, class allocator_type>
void swap (plf::queue<element_type, q_priority, allocator_type> &a, plf::queue<element_type, q_priority, allocator_type> &b) PLF_NOEXCEPT_SWAP(allocator_type)
{
	a.swap(b);
}

}


#undef PLF_MIN_BLOCK_CAPACITY
#undef PLF_MAX_BLOCK_CAPACITY

#undef PLF_EXCEPTIONS_SUPPORT
#undef PLF_DEFAULT_TEMPLATE_ARGUMENT_SUPPORT
#undef PLF_ALIGNMENT_SUPPORT
#undef PLF_INITIALIZER_LIST_SUPPORT
#undef PLF_TYPE_TRAITS_SUPPORT
#undef PLF_ALLOCATOR_TRAITS_SUPPORT
#undef PLF_VARIADICS_SUPPORT
#undef PLF_MOVE_SEMANTICS_SUPPORT
#undef PLF_NOEXCEPT
#undef PLF_NOEXCEPT_ALLOCATOR
#undef PLF_NOEXCEPT_SWAP
#undef PLF_NOEXCEPT_MOVE_ASSIGN
#undef PLF_CONSTEXPR
#undef PLF_CONSTFUNC
#undef PLF_CPP20_SUPPORT

#undef PLF_CONSTRUCT
#undef PLF_DESTROY
#undef PLF_ALLOCATE
#undef PLF_DEALLOCATE

#endif // PLF_QUEUE_H
