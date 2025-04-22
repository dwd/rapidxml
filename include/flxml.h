#ifndef RAPIDXML_HPP_INCLUDED
#define RAPIDXML_HPP_INCLUDED

// Copyright (C) 2006, 2009 Marcin Kalicinski
// Version 1.13
// Revision $DateTime: 2009/05/13 01:46:17 $
//! \file rapidxml.hpp This file contains rapidxml parser and DOM implementation

#include <flxml/wrappers.h>
#include <flxml/tables.h>

#include <cstdint>      // For std::size_t
#include <cassert>      // For assert
#include <new>          // For placement new
#include <string>
#include <span>
#include <optional>
#include <memory>
#include <stdexcept>    // For std::runtime_error

// On MSVC, disable "conditional expression is constant" warning (level 4).
// This warning is almost impossible to avoid with certain types of templated code
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable:4127)   // Conditional expression is constant
#endif

///////////////////////////////////////////////////////////////////////////
// RAPIDXML_PARSE_ERROR

#if defined(FLXML_NO_EXCEPTIONS)

#define FLXML_PARSE_ERROR(what, where) { parse_error_handler(what, where); assert(0); }
#define FLML_EOF_ERROR(what, where) { parse_error_handler(what, where); assert(0); }

namespace flxml
{
    //! When exceptions are disabled by defining RAPIDXML_NO_EXCEPTIONS,
    //! this function is called to notify user about the error.
    //! It must be defined by the user.
    //! <br><br>
    //! This function cannot return. If it does, the results are undefined.
    //! <br><br>
    //! A very simple definition might look like that:
    //! <pre>
    //! void %rapidxml::%parse_error_handler(const char *what, void *where)
    //! {
    //!     std::cout << "Parse error: " << what << "\n";
    //!     std::abort();
    //! }
    //! </pre>
    //! \param what Human readable description of the error.
    //! \param where Pointer to character data where error was detected.
    void parse_error_handler(const char *what, void *where);
}

#else

#define FLXML_PARSE_ERROR(what, where) {if (*where == Ch(0)) throw eof_error(what, nullptr); else throw parse_error(what, nullptr);} (void)0
#define FLXML_EOF_ERROR(what, where) throw eof_error(what, nullptr)

namespace flxml
{

    //! Parse error exception.
    //! This exception is thrown by the parser when an error occurs.
    //! Use what() function to get human-readable error message.
    //! Use where() function to get a pointer to position within source text where error was detected.
    //! <br><br>
    //! If throwing exceptions by the parser is undesirable,
    //! it can be disabled by defining RAPIDXML_NO_EXCEPTIONS macro before rapidxml.hpp is included.
    //! This will cause the parser to call rapidxml::parse_error_handler() function instead of throwing an exception.
    //! This function must be defined by the user.
    //! <br><br>
    //! This class derives from <code>std::exception</code> class.
    class parse_error: public std::runtime_error
    {

    public:

        //! Constructs parse error
        parse_error(const char *what, void *where)
            : std::runtime_error(what)
            , m_where(where)
        {
        }

        //! Gets pointer to character data where error happened.
        //! Ch should be the same as char type of xml_document that produced the error.
        //! \return Pointer to location within the parsed string where error occured.
        template<class Ch>
        Ch *where() const
        {
            return reinterpret_cast<Ch *>(m_where);
        }

    private:
        void *m_where;
    };

    class eof_error : public parse_error {
    public:
        using parse_error::parse_error;
    };

    class validation_error : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };

    class xmlns_unbound : public validation_error {
    public:
        using validation_error::validation_error;
    };

    class duplicate_attribute : public validation_error {
    public:
        using validation_error::validation_error;
    };

    class attr_xmlns_unbound : public xmlns_unbound {
    public:
        using xmlns_unbound::xmlns_unbound;
    };

    class element_xmlns_unbound : public xmlns_unbound {
    public:
        using xmlns_unbound::xmlns_unbound;
    };
}

#endif

///////////////////////////////////////////////////////////////////////////
// Pool sizes

#ifndef FLXML_STATIC_POOL_SIZE
    // Size of static memory block of memory_pool.
    // Define RAPIDXML_STATIC_POOL_SIZE before including rapidxml.hpp if you want to override the default value.
    // No dynamic memory allocations are performed by memory_pool until static memory is exhausted.
    #define FLXML_STATIC_POOL_SIZE (64 * 1024)
#endif

#ifndef FLXML_DYNAMIC_POOL_SIZE
    // Size of dynamic memory block of memory_pool.
    // Define RAPIDXML_DYNAMIC_POOL_SIZE before including rapidxml.hpp if you want to override the default value.
    // After the static block is exhausted, dynamic blocks with approximately this size are allocated by memory_pool.
    #define FLXML_DYNAMIC_POOL_SIZE (64 * 1024)
#endif

namespace flxml
{
    // Forward declarations
    template<typename Ch> class xml_node;
    template<typename Ch> class xml_attribute;
    template<typename Ch> class xml_document;
    template<typename Ch> class children;
    template<typename Ch> class descendants;
    template<typename Ch> class attributes;

    //! Enumeration listing all node types produced by the parser.
    //! Use xml_node::type() function to query node type.
    enum class node_type
    {
        node_document,      //!< A document node. Name and value are empty.
        node_element,       //!< An element node. Name contains element name. Value contains text of first data node.
        node_data,          //!< A data node. Name is empty. Value contains data text.
        node_cdata,         //!< A CDATA node. Name is empty. Value contains data text.
        node_comment,       //!< A comment node. Name is empty. Value contains comment text.
        node_declaration,   //!< A declaration node. Name and value are empty. Declaration parameters (version, encoding and standalone) are in node attributes.
        node_doctype,       //!< A DOCTYPE node. Name is empty. Value contains DOCTYPE text.
        node_pi,            //!< A PI node. Name contains target. Value contains instructions.
	    node_literal        //!< Value is unencoded text (used for inserting pre-rendered XML).
    };
    using enum node_type; // Import this into the rapidxml namespace as before.

    ///////////////////////////////////////////////////////////////////////
    // Parsing flags

    //! Parse flag instructing the parser to not create data nodes.
    //! Text of first data node will still be placed in value of parent element, unless rapidxml::parse_no_element_values flag is also specified.
    //! Can be combined with other flags by use of | operator.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_no_data_nodes = 0x1;

    //! Parse flag instructing the parser to not use text of first data node as a value of parent element.
    //! Can be combined with other flags by use of | operator.
    //! Note that child data nodes of element node take precendence over its value when printing.
    //! That is, if element has one or more child data nodes <em>and</em> a value, the value will be ignored.
    //! Use rapidxml::parse_no_data_nodes flag to prevent creation of data nodes if you want to manipulate data using values of elements.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_no_element_values = 0x2;

    //! Parse flag instructing the parser to not translate entities in the source text.
    //! By default entities are translated, modifying source text.
    //! Can be combined with other flags by use of | operator.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_no_entity_translation = 0x8;

    //! Parse flag instructing the parser to disable UTF-8 handling and assume plain 8 bit characters.
    //! By default, UTF-8 handling is enabled.
    //! Can be combined with other flags by use of | operator.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_no_utf8 = 0x10;

    //! Parse flag instructing the parser to create XML declaration node.
    //! By default, declaration node is not created.
    //! Can be combined with other flags by use of | operator.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_declaration_node = 0x20;

    //! Parse flag instructing the parser to create comments nodes.
    //! By default, comment nodes are not created.
    //! Can be combined with other flags by use of | operator.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_comment_nodes = 0x40;

    //! Parse flag instructing the parser to create DOCTYPE node.
    //! By default, doctype node is not created.
    //! Although W3C specification allows at most one DOCTYPE node, RapidXml will silently accept documents with more than one.
    //! Can be combined with other flags by use of | operator.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_doctype_node = 0x80;

    //! Parse flag instructing the parser to create PI nodes.
    //! By default, PI nodes are not created.
    //! Can be combined with other flags by use of | operator.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_pi_nodes = 0x100;

    //! Parse flag instructing the parser to validate closing tag names.
    //! If not set, name inside closing tag is irrelevant to the parser.
    //! By default, closing tags are not validated.
    //! Can be combined with other flags by use of | operator.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_validate_closing_tags = 0x200;

    //! Parse flag instructing the parser to trim all leading and trailing whitespace of data nodes.
    //! By default, whitespace is not trimmed.
    //! This flag does not cause the parser to modify source text.
    //! Can be combined with other flags by use of | operator.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_trim_whitespace = 0x400;

    //! Parse flag instructing the parser to condense all whitespace runs of data nodes to a single space character.
    //! Trimming of leading and trailing whitespace of data is controlled by rapidxml::parse_trim_whitespace flag.
    //! By default, whitespace is not normalized.
    //! If this flag is specified, source text will be modified.
    //! Can be combined with other flags by use of | operator.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_normalize_whitespace = 0x800;

    //! Parse flag to say "Parse only the initial element opening."
    //! Useful for XMLstreams used in XMPP.
    const int parse_open_only = 0x1000;

    //! Parse flag to say "Toss the children of the top node and parse off
    //! one element.
    //! Useful for parsing off XMPP top-level elements.
    const int parse_parse_one = 0x2000;

    //! Parse flag to say "Validate XML namespaces fully."
    //! This will generate additional errors, including unbound prefixes
    //! and duplicate attributes (with different prefices)
    const int parse_validate_xmlns = 0x4000;

    // Compound flags

    //! Parse flags which represent default behaviour of the parser.
    //! This is always equal to 0, so that all other flags can be simply ored together.
    //! Normally there is no need to inconveniently disable flags by anding with their negated (~) values.
    //! This also means that meaning of each flag is a <i>negation</i> of the default setting.
    //! For example, if flag name is rapidxml::parse_no_utf8, it means that utf-8 is <i>enabled</i> by default,
    //! and using the flag will disable it.
    //! <br><br>
    //! See xml_document::parse() function.
    [[maybe_unused]] const int parse_default = 0;

    //! A combination of parse flags resulting in fastest possible parsing, without sacrificing important data.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_fastest = parse_no_data_nodes;

    //! A combination of parse flags resulting in largest amount of data being extracted.
    //! This usually results in slowest parsing.
    //! <br><br>
    //! See xml_document::parse() function.
    const int parse_full = parse_declaration_node | parse_comment_nodes | parse_doctype_node | parse_pi_nodes | parse_validate_closing_tags | parse_validate_xmlns;


    ///////////////////////////////////////////////////////////////////////
    // Memory pool

    //! This class is used by the parser to create new nodes and attributes, without overheads of dynamic memory allocation.
    //! In most cases, you will not need to use this class directly.
    //! However, if you need to create nodes manually or modify names/values of nodes,
    //! you are encouraged to use memory_pool of relevant xml_document to allocate the memory.
    //! Not only is this faster than allocating them by using <code>new</code> operator,
    //! but also their lifetime will be tied to the lifetime of document,
    //! possibly simplyfing memory management.
    //! <br><br>
    //! Call allocate_node() or allocate_attribute() functions to obtain new nodes or attributes from the pool.
    //! You can also call allocate_string() function to allocate strings.
    //! Such strings can then be used as names or values of nodes without worrying about their lifetime.
    //! Note that there is no <code>free()</code> function -- all allocations are freed at once when clear() function is called,
    //! or when the pool is destroyed.
    //! <br><br>
    //! It is also possible to create a standalone memory_pool, and use it
    //! to allocate nodes, whose lifetime will not be tied to any document.
    //! <br><br>
    //! Pool maintains <code>RAPIDXML_STATIC_POOL_SIZE</code> bytes of statically allocated memory.
    //! Until static memory is exhausted, no dynamic memory allocations are done.
    //! When static memory is exhausted, pool allocates additional blocks of memory of size <code>RAPIDXML_DYNAMIC_POOL_SIZE</code> each,
    //! by using global <code>new[]</code> and <code>delete[]</code> operators.
    //! This behaviour can be changed by setting custom allocation routines.
    //! Use set_allocator() function to set them.
    //! <br><br>
    //! Allocations for nodes, attributes and strings are aligned at <code>RAPIDXML_ALIGNMENT</code> bytes.
    //! This value defaults to the size of pointer on target architecture.
    //! <br><br>
    //! To obtain absolutely top performance from the parser,
    //! it is important that all nodes are allocated from a single, contiguous block of memory.
    //! Otherwise, cache misses when jumping between two (or more) disjoint blocks of memory can slow down parsing quite considerably.
    //! If required, you can tweak <code>RAPIDXML_STATIC_POOL_SIZE</code>, <code>RAPIDXML_DYNAMIC_POOL_SIZE</code> and <code>RAPIDXML_ALIGNMENT</code>
    //! to obtain best wasted memory to performance compromise.
    //! To do it, define their values before rapidxml.hpp file is included.
    //! \param Ch Character type of created nodes.
    template<typename Ch = char>
    class memory_pool
    {

    public:

        //! \cond internal
        using alloc_func = void * (*)(std::size_t);       // Type of user-defined function used to allocate memory
        using free_func = void (*)(void *);              // Type of user-defined function used to free memory
        //! \endcond

        //! Constructs empty pool with default allocator functions.
        memory_pool() {
            init();
        }
        memory_pool(memory_pool const &) = delete;
        memory_pool(memory_pool &&) = delete;

        //! Destroys pool and frees all the memory.
        //! This causes memory occupied by nodes allocated by the pool to be freed.
        //! Nodes allocated from the pool are no longer valid.
        ~memory_pool()
        {
            clear();
        }

        using view_type = std::basic_string_view<Ch>;

        //! Allocates a new node from the pool, and optionally assigns name and value to it.
        //! If the allocation request cannot be accomodated, this function will throw <code>std::bad_alloc</code>.
        //! If exceptions are disabled by defining RAPIDXML_NO_EXCEPTIONS, this function
        //! will call rapidxml::parse_error_handler() function.
        //! \param type Type of node to create.
        //! \param name Name to assign to the node, or 0 to assign no name.
        //! \param value Value to assign to the node, or 0 to assign no value.
        //! \param name_size Size of name to assign, or 0 to automatically calculate size from name string.
        //! \param value_size Size of value to assign, or 0 to automatically calculate size from value string.
        //! \return Pointer to allocated node. This pointer will never be NULL.
        template<typename... Args>
        xml_node<Ch> * allocate_node_low(Args... args) {
            void *memory = allocate_aligned<xml_node<Ch>>();
            auto *node = new(memory) xml_node<Ch>(args...);
            return node;
        }
        xml_node<Ch> * allocate_node(node_type type, view_type const & name, view_type const & value) {
            auto * node = this->allocate_node_low(type, name);
            node->value(value);
            return node;
        }
        xml_node<Ch> * allocate_node(node_type type, view_type const & name) {
            return this->allocate_node_low(type, name);
        }
        xml_node<Ch> * allocate_node(node_type type) {
            return this->allocate_node_low(type);
        }

        //! Allocates a new attribute from the pool, and optionally assigns name and value to it.
        //! If the allocation request cannot be accomodated, this function will throw <code>std::bad_alloc</code>.
        //! If exceptions are disabled by defining RAPIDXML_NO_EXCEPTIONS, this function
        //! will call rapidxml::parse_error_handler() function.
        //! \param name Name to assign to the attribute, or 0 to assign no name.
        //! \param value Value to assign to the attribute, or 0 to assign no value.
        //! \param name_size Size of name to assign, or 0 to automatically calculate size from name string.
        //! \param value_size Size of value to assign, or 0 to automatically calculate size from value string.
        //! \return Pointer to allocated attribute. This pointer will never be NULL.
        template<typename... Args>
        xml_attribute<Ch> *allocate_attribute_low(Args... args) {
            void *memory = allocate_aligned<xml_attribute<Ch>>();
            auto *attribute = new(memory) xml_attribute<Ch>(args...);
            return attribute;
        }
        xml_attribute<Ch> * allocate_attribute(view_type const & name, view_type const & value) {
            auto * attr = this->allocate_attribute_low(name);
            attr->value(value);
            return attr;
        }
        xml_attribute<Ch> * allocate_attribute(view_type const & name) {
            return this->allocate_attribute_low(name);
        }
        xml_attribute<Ch> * allocate_attribute() {
            return this->allocate_attribute_low();
        }

        //! Allocates a char array of given size from the pool, and optionally copies a given string to it.
        //! If the allocation request cannot be accomodated, this function will throw <code>std::bad_alloc</code>.
        //! If exceptions are disabled by defining RAPIDXML_NO_EXCEPTIONS, this function
        //! will call rapidxml::parse_error_handler() function.
        //! \param source String to initialize the allocated memory with, or 0 to not initialize it.
        //! \param size Number of characters to allocate, or zero to calculate it automatically from source string length; if size is 0, source string must be specified and null terminated.
        //! \return Pointer to allocated char array. This pointer will never be NULL.
        template<typename Sch>
        std::span<Ch> allocate_span(std::basic_string_view<Sch> const & source)
        {
            if (source.size() == 0) return {}; // No need to allocate.
            Ch *result = allocate_aligned<Ch>(source.size());
            for (std::size_t i = 0; i < source.size(); ++i)
                result[i] = source[i];
            return {result, source.size()};
        }

        template<typename Sch>
        view_type allocate_string(std::basic_string_view<Sch> const & source) {
            auto span = allocate_span(source);
            return {span.data(), span.size()};
        }

        template<typename Sch>
        view_type allocate_string(std::basic_string<Sch> const & source) {
            return allocate_string(std::basic_string_view{source.data(), source.size()});
        }

        template<typename Sch>
        view_type allocate_string(const Sch * source) {
            return allocate_string(std::basic_string_view<Sch>(source));
        }

        view_type const & nullstr()
        {
            return m_nullstr;
        }
        view_type const & xmlns_xml()
        {
            if (m_xmlns_xml.empty())
                m_xmlns_xml = allocate_string("http://www.w3.org/XML/1998/namespace");
            return m_xmlns_xml;
        }
        view_type const & xmlns_xmlns()
        {
            if (m_xmlns_xmlns.empty())
                m_xmlns_xmlns = allocate_string("http://www.w3.org/2000/xmlns/");
            return m_xmlns_xmlns;
        }


        //! Clones an xml_node and its hierarchy of child nodes and attributes.
        //! Nodes and attributes are allocated from this memory pool.
        //! Names and values are not cloned, they are shared between the clone and the source.
        //! Result node can be optionally specified as a second parameter,
        //! in which case its contents will be replaced with cloned source node.
        //! This is useful when you want to clone entire document.
        //! \param source Node to clone.
        //! \param result Node to put results in, or 0 to automatically allocate result node
        //! \return Pointer to cloned node. This pointer will never be NULL.
        optional_ptr<xml_node<Ch>> clone_node(const optional_ptr<xml_node<Ch>> source, bool strings=false)
        {
            // Prepare result node
            auto result = allocate_node(source->type());
            auto s = [this, strings](view_type const & sv) { return strings ? this->allocate_string(sv) : sv; };

            // Clone name and value
            result->name(s(source->name()));
            result->value(s(source->value()));
            result->prefix(s(source->prefix()));

            // Clone child nodes and attributes
            for (auto child = source->first_node(); child; child = child->next_sibling())
                result->append_node(clone_node(child, strings));
            for (auto attr = source->first_attribute(); attr; attr = attr->next_attribute())
                result->append_attribute(allocate_attribute(s(attr->name()), s(attr->value())));

            return result;
        }

        //! Clears the pool.
        //! This causes memory occupied by nodes allocated by the pool to be freed.
        //! Any nodes or strings allocated from the pool will no longer be valid.
        void clear()
        {
            while (m_begin != m_static_memory.data())
            {
                std::size_t s = sizeof(header) * 2;
                void * h = m_begin;
                std::align(alignof(header), sizeof(header), h, s);
                void *previous_begin = reinterpret_cast<header *>(h)->previous_begin;
                if (m_free_func)
                    m_free_func(m_begin);
                else
                    delete[] reinterpret_cast<char *>(m_begin);
                m_begin = previous_begin;
            }
            init();
        }

        //! Sets or resets the user-defined memory allocation functions for the pool.
        //! This can only be called when no memory is allocated from the pool yet, otherwise results are undefined.
        //! Allocation function must not return invalid pointer on failure. It should either throw,
        //! stop the program, or use <code>longjmp()</code> function to pass control to other place of program.
        //! If it returns invalid pointer, results are undefined.
        //! <br><br>
        //! User defined allocation functions must have the following forms:
        //! <br><code>
        //! <br>void *allocate(std::size_t size);
        //! <br>void free(void *pointer);
        //! </code><br>
        //! \param af Allocation function, or 0 to restore default function
        //! \param ff Free function, or 0 to restore default function
        [[maybe_unused]] void set_allocator(alloc_func af, free_func ff)
        {
            assert(m_begin == m_static_memory.data() && m_ptr == m_begin);    // Verify that no memory is allocated yet
            m_alloc_func = af;
            m_free_func = ff;
        }

    private:

        struct header
        {
            void *previous_begin;
        };

        void init()
        {
            m_begin = m_static_memory.data();
            m_ptr = m_begin;
            m_space = m_static_memory.size();
        }

        void *allocate_raw(std::size_t size)
        {
            // Allocate
            void *memory;
            if (m_alloc_func)   // Allocate memory using either user-specified allocation function or global operator new[]
            {
                memory = m_alloc_func(size);
                assert(memory); // Allocator is not allowed to return 0, on failure it must either throw, stop the program or use longjmp
            }
            else
            {
                memory = new char[size];
#ifdef FLXML_NO_EXCEPTIONS
                if (!memory)            // If exceptions are disabled, verify memory allocation, because new will not be able to throw bad_alloc
                    FLXML_PARSE_ERROR("out of memory", 0);
#endif
            }
            return memory;
        }

        template<typename T>
        T *allocate_aligned(std::size_t n = 1)
        {
            auto size = n * sizeof(T);
            // Calculate aligned pointer
            if (!std::align(alignof(T), sizeof(T) * n, m_ptr, m_space)) {
                // If not enough memory left in current pool, allocate a new pool
                // Calculate required pool size (may be bigger than RAPIDXML_DYNAMIC_POOL_SIZE)
                std::size_t pool_size = FLXML_DYNAMIC_POOL_SIZE;
                if (pool_size < size)
                    pool_size = size;

                // Allocate
                std::size_t alloc_size = sizeof(header) + (2 * alignof(header) - 2) + pool_size;     // 2 alignments required in worst case: one for header, one for actual allocation
                void *raw_memory = allocate_raw(alloc_size);

                // Setup new pool in allocated memory
                void *new_header = raw_memory;
                std::align(alignof(header), sizeof(header), new_header, alloc_size);
                auto * h = reinterpret_cast<header *>(new_header);
                h->previous_begin = m_begin;
                m_begin = raw_memory;
                m_ptr = (h + 1);
                m_space = alloc_size - sizeof(header);

                // Calculate aligned pointer again using new pool
                return allocate_aligned<T>(n);
            }
            auto * result = reinterpret_cast<T *>(m_ptr);
            m_ptr = (result + n);
            m_space -= size;
            auto blank = reinterpret_cast<char *>(result);
            auto end = blank + size;
            while (blank != end) *blank++ = 'X';
            return result;
        }

        void *m_begin = nullptr;                                      // Start of raw memory making up current pool
        void *m_ptr = nullptr;                                        // First free byte in current pool
        std::size_t m_space = FLXML_STATIC_POOL_SIZE;                                        // Available space remaining
        std::array<char, FLXML_STATIC_POOL_SIZE> m_static_memory = {};    // Static raw memory
        alloc_func m_alloc_func = nullptr;                           // Allocator function, or 0 if default is to be used
        free_func m_free_func = nullptr;                             // Free function, or 0 if default is to be used
        view_type m_nullstr;
        view_type m_xmlns_xml;
        view_type m_xmlns_xmlns;
    };

    ///////////////////////////////////////////////////////////////////////////
    // XML base

    //! Base class for xml_node and xml_attribute implementing common functions:
    //! name(), name_size(), value(), value_size() and parent().
    //! \param Ch Character type to use
    template<typename Ch = char>
    class xml_base
    {

    public:
        using view_type = std::basic_string_view<Ch>;

        ///////////////////////////////////////////////////////////////////////////
        // Construction & destruction

        // Construct a base with empty name, value and parent
        xml_base() = default;
        explicit xml_base(view_type const & name) : m_name(name) {}
        xml_base(view_type const & name, view_type const & value) : m_name(name), m_value(value) {}

        ///////////////////////////////////////////////////////////////////////////
        // Node data access

        //! Gets name of the node.
        //! Interpretation of name depends on type of node.
        //! Note that name will not be zero-terminated if rapidxml::parse_no_string_terminators option was selected during parse.
        //! <br><br>
        //! Use name_size() function to determine length of the name.
        //! \return Name of node, or empty string if node has no name.
        view_type const & name() const
        {
            return m_name;
        }

        //! Gets value of node.
        //! Interpretation of value depends on type of node.
        //! Note that value will not be zero-terminated if rapidxml::parse_no_string_terminators option was selected during parse.
        //! <br><br>
        //! Use value_size() function to determine length of the value.
        //! \return Value of node, or empty string if node has no value.
        view_type const & value_raw() const
        {
            return m_value;
        }

        ///////////////////////////////////////////////////////////////////////////
        // Node modification

        //! Sets name of node to a non zero-terminated string.
        //! See \ref ownership_of_strings.
        //! <br><br>
        //! Note that node does not own its name or value, it only stores a pointer to it.
        //! It will not delete or otherwise free the pointer on destruction.
        //! It is reponsibility of the user to properly manage lifetime of the string.
        //! The easiest way to achieve it is to use memory_pool of the document to allocate the string -
        //! on destruction of the document the string will be automatically freed.
        //! <br><br>
        //! Size of name must be specified separately, because name does not have to be zero terminated.
        //! Use name(const Ch *) function to have the length automatically calculated (string must be zero terminated).
        //! \param name Name of node to set. Does not have to be zero terminated.
        //! \param size Size of name, in characters. This does not include zero terminator, if one is present.
        void name(view_type const & name) {
            m_name = name;
        }

        //! Sets value of node to a non zero-terminated string.
        //! See \ref ownership_of_strings.
        //! <br><br>
        //! Note that node does not own its name or value, it only stores a pointer to it.
        //! It will not delete or otherwise free the pointer on destruction.
        //! It is reponsibility of the user to properly manage lifetime of the string.
        //! The easiest way to achieve it is to use memory_pool of the document to allocate the string -
        //! on destruction of the document the string will be automatically freed.
        //! <br><br>
        //! Size of value must be specified separately, because it does not have to be zero terminated.
        //! Use value(const Ch *) function to have the length automatically calculated (string must be zero terminated).
        //! <br><br>
        //! If an element has a child node of type node_data, it will take precedence over element value when printing.
        //! If you want to manipulate data of elements using values, use parser flag rapidxml::parse_no_data_nodes to prevent creation of data nodes by the parser.
        //! \param value value of node to set. Does not have to be zero terminated.
        //! \param size Size of value, in characters. This does not include zero terminator, if one is present.
        void value_raw(view_type const & value)
        {
            m_value = value;
        }

        ///////////////////////////////////////////////////////////////////////////
        // Related nodes access

        //! Gets node parent.
        //! \return Pointer to parent node, or 0 if there is no parent.
        optional_ptr<xml_node<Ch>> parent() const
        {
            return m_parent;
        }

    protected:
        view_type m_name;                         // Name of node, or 0 if no name
        view_type m_value;                        // Value of node, or 0 if no value
        xml_node<Ch> *m_parent = nullptr;             // Pointer to parent node, or 0 if none
    };

    //! Class representing attribute node of XML document.
    //! Each attribute has name and value strings, which are available through name() and value() functions (inherited from xml_base).
    //! Note that after parse, both name and value of attribute will point to interior of source text used for parsing.
    //! Thus, this text must persist in memory for the lifetime of attribute.
    //! \param Ch Character type to use.
    template<typename Ch = char>
    class xml_attribute: public xml_base<Ch>
    {

        friend class xml_node<Ch>;

    public:
        using view_type = std::basic_string_view<Ch>;
        using ptr = optional_ptr<xml_attribute<Ch>>;

        ///////////////////////////////////////////////////////////////////////////
        // Construction & destruction

        //! Constructs an empty attribute with the specified type.
        //! Consider using memory_pool of appropriate xml_document if allocating attributes manually.
        xml_attribute() = default;
        xml_attribute(view_type const & name) : xml_base<Ch>(name) {}
        xml_attribute(view_type const & name, view_type const & value) : xml_base<Ch>(name, value) {}

        void quote(Ch q) {
            m_quote = q;
        }
        Ch quote() const {
            return m_quote;
        }

        view_type const & value() const {
            if (m_value.has_value()) return m_value.value();
            m_value = document()->decode_attr_value(this);
            return m_value.value();
        }
        void value(view_type const & v) {
            m_value = v;
            this->value_raw("");
            if (this->m_parent) this->m_parent->dirty_parent();
        }
        // Return true if the value has been decoded.
        bool value_decoded() const {
            // Either we don't have a decoded value, or we do but it's identical.
            return !m_value.has_value() || m_value.value().data() != this->value_raw().data();
        }

        ///////////////////////////////////////////////////////////////////////////
        // Related nodes access

        //! Gets document of which attribute is a child.
        //! \return Pointer to document that contains this attribute, or 0 if there is no parent document.
        optional_ptr<xml_document<Ch>> document() const {
            if (auto node = this->parent()) {
                return node->document();
            } else {
                return nullptr;
            }
        }

        view_type const & xmlns() const {
            if (m_xmlns.has_value()) return m_xmlns.value();
            auto const & name = this->name();
            auto colon = name.find(':');
            if (colon != view_type::npos) {
                auto element = this->parent();
                if (element) m_xmlns = element->xmlns_lookup(name.substr(0, colon), true);
            } else {
                m_xmlns = document()->nullstr();
            }
            return m_xmlns.value();
        }
        //! Gets previous attribute, optionally matching attribute name.
        //! \param name Name of attribute to find, or 0 to return previous attribute regardless of its name; this string doesn't have to be zero-terminated if name_size is non-zero
        //! \param name_size Size of name, in characters, or 0 to have size calculated automatically from string
        //! \param case_sensitive Should name comparison be case-sensitive; non case-sensitive comparison works properly only for ASCII characters
        //! \return Pointer to found attribute, or 0 if not found.
        optional_ptr<xml_attribute<Ch>> previous_attribute(view_type const & name = {}) const
        {
            if (name)
            {
                for (xml_attribute<Ch> *attribute = m_prev_attribute; attribute; attribute = attribute->m_prev_attribute)
                    if (name == attribute->name())
                        return attribute;
                return 0;
            }
            else
                return this->m_parent ? m_prev_attribute : 0;
        }

        //! Gets next attribute, optionally matching attribute name.
        //! \param name Name of attribute to find, or 0 to return next attribute regardless of its name; this string doesn't have to be zero-terminated if name_size is non-zero
        //! \param name_size Size of name, in characters, or 0 to have size calculated automatically from string
        //! \param case_sensitive Should name comparison be case-sensitive; non case-sensitive comparison works properly only for ASCII characters
        //! \return Pointer to found attribute, or 0 if not found.
        optional_ptr<xml_attribute<Ch>> next_attribute(view_type const & name = {}) const
        {
            if (!name.empty())
            {
                for (xml_attribute<Ch> *attribute = m_next_attribute; attribute; attribute = attribute->m_next_attribute)
                    if (attribute->name() == name)
                        return attribute;
                return nullptr;
            }
            else
                return this->m_parent ? m_next_attribute : nullptr;
        }

        view_type const & local_name() const
        {
            if (!m_local_name.empty()) return m_local_name;
            auto colon = this->name().find(':');
            if (colon == view_type::npos) {
                m_local_name = this->name();
            } else {
                m_local_name = this->name().substr(colon + 1);
            }
            return m_local_name;
        }

    private:

        xml_attribute<Ch> *m_prev_attribute = nullptr;        // Pointer to previous sibling of attribute, or 0 if none; only valid if parent is non-zero
        xml_attribute<Ch> *m_next_attribute = nullptr;        // Pointer to next sibling of attribute, or 0 if none; only valid if parent is non-zero
        Ch m_quote = 0; // When parsing, this should be set to the containing quote for the value.
        mutable std::optional<view_type> m_xmlns;
        mutable std::optional<view_type> m_value; // This is the decoded, not raw, value.
        mutable view_type m_local_name; // ATTN: points inside m_name.
    };

    ///////////////////////////////////////////////////////////////////////////
    // XML node

    //! Class representing a node of XML document.
    //! Each node may have associated name and value strings, which are available through name() and value() functions.
    //! Interpretation of name and value depends on type of the node.
    //! Type of node can be determined by using type() function.
    //! <br><br>
    //! Note that after parse, both name and value of node, if any, will point interior of source text used for parsing.
    //! Thus, this text must persist in the memory for the lifetime of node.
    //! \param Ch Character type to use.
    template<typename Ch = char>
    class xml_node: public xml_base<Ch>
    {
    public:
        using view_type = std::basic_string_view<Ch>;
        using ptr = optional_ptr<xml_node<Ch>>;

        ///////////////////////////////////////////////////////////////////////////
        // Construction & destruction

        //! Constructs an empty node with the specified type.
        //! Consider using memory_pool of appropriate document to allocate nodes manually.
        //! \param type Type of node to construct.
        explicit xml_node(node_type type)
            : m_type(type)
        {
        }
        xml_node(node_type type, view_type const & name) : xml_base<Ch>(name), m_type(type) {}
        xml_node(node_type type, view_type const & name, view_type const & value) : xml_base<Ch>(name, value), m_type(type) {}

        ///////////////////////////////////////////////////////////////////////////
        // Node data access
        view_type const & value() const {
            if (m_value.has_value()) return m_value.value();
            if (m_type == node_element || m_type == node_data) {
                m_value = document()->decode_data_value(this);
            } else {
                m_value = this->value_raw();
            }
            return m_value.value();
        }

        void dirty() {
            m_clean = false;
            dirty_parent();
        }
        void dirty_parent() {
            if (this->m_parent) this->m_parent->dirty();
        }
        bool clean() const {
            return m_clean;
        }

        void value(view_type const & v) {
            if (this->m_type == node_element) {
                // Set the first data node to the value, if one exists.
                for (auto node = m_first_node; node; node = node->m_next_sibling) {
                    if (node->type() == node_data) {
                        node->value(v);
                        break;
                    }
                }
            }
            m_value = v;
            this->value_raw("");
            dirty();
        }

        bool value_decoded() const {
            return !m_value.has_value() || m_value.value().data() != this->value_raw().data();
        }

        //! Gets type of node.
        //! \return Type of node.
        node_type type() const {
            return m_type;
        }

        void prefix(view_type const & prefix) {
            m_prefix = prefix;
            dirty_parent();
        }

        view_type const & prefix() const {
            return m_prefix;
        }

        void contents(view_type const & contents) {
            m_contents = contents;
            // Reset to clean here.
            m_clean = true;
        }
        view_type const & contents() const
        {
            return m_contents;
        }

        view_type const & xmlns() const {
            if (m_xmlns.has_value()) return m_xmlns.value();
            m_xmlns = xmlns_lookup(m_prefix, false);
            return m_xmlns.value();
        }

        view_type const & xmlns_lookup(view_type const & prefix, bool attribute) const
        {
            std::basic_string<Ch> attrname{"xmlns"};
            if (!prefix.empty()) {
                // Check if the prefix begins "xml".
                if (prefix.size() >= 3 && prefix.starts_with("xml")) {
                    if (prefix.size() == 3) {
                        return this->document()->xmlns_xml();
                    } else if (prefix.size() == 5
                               && prefix[3] == Ch('n')
                               && prefix[4] == Ch('s')) {
                        return this->document()->xmlns_xmlns();
                    }
                }
                attrname += ':';
                attrname += prefix;
            }
            for (const xml_node<Ch> * node = this;
                 node;
                 node = node->m_parent) {
                auto attr = node->first_attribute(attrname);
                if (attr) {
                   return attr->value();
                }
            }
            if (!prefix.empty()) {
                if (attribute) {
                    throw attr_xmlns_unbound(attrname.c_str());
                } else {
                    throw element_xmlns_unbound(attrname.c_str());
                }
            }
            return document()->nullstr();
        }

        ///////////////////////////////////////////////////////////////////////////
        // Related nodes access

        //! Gets document of which node is a child.
        //! \return Pointer to document that contains this node, or 0 if there is no parent document.
        optional_ptr<xml_document<Ch>> document() const
        {
            auto *node = this;
            while (node) {
                if (node->type() == node_document) {
                    return static_cast<xml_document<Ch> *>(const_cast<xml_node<Ch> *>(node));
                }
                node = node->parent().ptr_unsafe();
            }
            return nullptr;
        }

        flxml::children<Ch> children() const {
            return flxml::children<Ch>{*this};
        }

        flxml::descendants<Ch> descendants() const {
            return flxml::descendants<Ch>{optional_ptr<xml_node<Ch>>{const_cast<xml_node<Ch> *>(this)}};
        }

        flxml::attributes<Ch> attributes() const {
            return flxml::attributes<Ch>{*this};
        }

        //! Gets first child node, optionally matching node name.
        //! \param name Name of child to find, or 0 to return first child regardless of its name; this string doesn't have to be zero-terminated if name_size is non-zero
        //! \param name_size Size of name, in characters, or 0 to have size calculated automatically from string
        //! \param case_sensitive Should name comparison be case-sensitive; non case-sensitive comparison works properly only for ASCII characters
        //! \return Pointer to found child, or 0 if not found.
        optional_ptr<xml_node<Ch>> first_node(view_type const & name = {}, view_type const & asked_xmlns = {}) const
        {
            view_type xmlns = asked_xmlns;
            if (asked_xmlns.empty() && !name.empty()) {
                // No XMLNS asked for, but a name is present.
                // Assume "same XMLNS".
                xmlns = this->xmlns();
            }
            for (xml_node<Ch> *child = m_first_node; child; child = child->m_next_sibling) {
                if ((name.empty() || child->name() == name)
                    && (xmlns.empty() || child->xmlns() == xmlns)) {
                    return child;
                }
            }
            return nullptr;
        }

        //! Gets last child node, optionally matching node name.
        //! Behaviour is undefined if node has no children.
        //! Use first_node() to test if node has children.
        //! \param name Name of child to find, or 0 to return last child regardless of its name; this string doesn't have to be zero-terminated if name_size is non-zero
        //! \param name_size Size of name, in characters, or 0 to have size calculated automatically from string
        //! \param case_sensitive Should name comparison be case-sensitive; non case-sensitive comparison works properly only for ASCII characters
        //! \return Pointer to found child, or 0 if not found.
        optional_ptr<xml_node<Ch>> last_node(view_type const & name = {}, view_type const & asked_xmlns = {}) const
        {
            view_type xmlns = asked_xmlns;
            if (asked_xmlns.empty() && !name.empty()) {
                // No XMLNS asked for, but a name is present.
                // Assume "same XMLNS".
                xmlns = this->xmlns();
            }
            for (xml_node<Ch> *child = m_last_node; child; child = child->m_prev_sibling) {
                if ((name.empty() || child->name() == name)
                    && (xmlns.empty() || child->xmlns() == xmlns)) {
                    return child;
                }
            }
            return nullptr;
        }

        //! Gets previous sibling node, optionally matching node name.
        //! Behaviour is undefined if node has no parent.
        //! Use parent() to test if node has a parent.
        //! \param name Name of sibling to find, or 0 to return previous sibling regardless of its name; this string doesn't have to be zero-terminated if name_size is non-zero
        //! \param name_size Size of name, in characters, or 0 to have size calculated automatically from string
        //! \param case_sensitive Should name comparison be case-sensitive; non case-sensitive comparison works properly only for ASCII characters
        //! \return Pointer to found sibling, or 0 if not found.
        optional_ptr<xml_node<Ch>> previous_sibling(view_type const & name = {}, view_type const & asked_xmlns = {}) const
        {
            assert(this->m_parent);     // Cannot query for siblings if node has no parent
            if (!name.empty())
            {
                view_type xmlns = asked_xmlns;
                if (xmlns.empty() && !name.empty()) {
                    // No XMLNS asked for, but a name is present.
                    // Assume "same XMLNS".
                    xmlns = this->xmlns();
                }
                for (xml_node<Ch> *sibling = m_prev_sibling; sibling; sibling = sibling->m_prev_sibling)
                    if ((name.empty() || sibling->name() == name)
                        && (xmlns.empty() || sibling->xmlns() == xmlns))
                        return sibling;
                return nullptr;
            }
            else
                return m_prev_sibling;
        }

        //! Gets next sibling node, optionally matching node name.
        //! Behaviour is undefined if node has no parent.
        //! Use parent() to test if node has a parent.
        //! \param name Name of sibling to find, or 0 to return next sibling regardless of its name; this string doesn't have to be zero-terminated if name_size is non-zero
        //! \param xmlns Namespace of sibling to find, or 0 to return next sibling regardless of its name; this string doesn't have to be zero-terminated if name_size is non-zero
        //! \param name_size Size of name, in characters, or 0 to have size calculated automatically from string
        //! \param case_sensitive Should name comparison be case-sensitive; non case-sensitive comparison works properly only for ASCII characters
        //! \return Pointer to found sibling, or 0 if not found.
        optional_ptr<xml_node<Ch>> next_sibling(view_type const & name = {}, view_type const & asked_xmlns = {}) const
        {
            assert(this->m_parent);     // Cannot query for siblings if node has no parent
            view_type xmlns = asked_xmlns;
            if (xmlns.empty() && !name.empty()) {
                // No XMLNS asked for, but a name is present.
                // Assume "same XMLNS".
                xmlns = this->xmlns();
            }
            for (xml_node<Ch> *sibling = m_next_sibling; sibling; sibling = sibling->m_next_sibling)
                if ((name.empty() || sibling->name() == name)
                    && (xmlns.empty() || sibling->xmlns() == xmlns))
                    return sibling;
            return nullptr;
        }

        //! Gets first attribute of node, optionally matching attribute name.
        //! \param name Name of attribute to find, or 0 to return first attribute regardless of its name; this string doesn't have to be zero-terminated if name_size is non-zero
        //! \param name_size Size of name, in characters, or 0 to have size calculated automatically from string
        //! \param case_sensitive Should name comparison be case-sensitive; non case-sensitive comparison works properly only for ASCII characters
        //! \return Pointer to found attribute, or 0 if not found.
        optional_ptr<xml_attribute<Ch>> first_attribute(view_type const & name = {}, view_type const & xmlns = {}) const
        {
            for (xml_attribute<Ch> *attribute = m_first_attribute; attribute; attribute = attribute->m_next_attribute)
                if ((name.empty() || attribute->name() == name) && (xmlns.empty() || attribute->xmlns() == xmlns))
                    return attribute;
            return nullptr;
        }

        //! Gets last attribute of node, optionally matching attribute name.
        //! \param name Name of attribute to find, or 0 to return last attribute regardless of its name; this string doesn't have to be zero-terminated if name_size is non-zero
        //! \param name_size Size of name, in characters, or 0 to have size calculated automatically from string
        //! \param case_sensitive Should name comparison be case-sensitive; non case-sensitive comparison works properly only for ASCII characters
        //! \return Pointer to found attribute, or 0 if not found.
        optional_ptr<xml_attribute<Ch>> last_attribute(view_type const & name = {}, view_type const & xmlns = {}) const
        {
            for (xml_attribute<Ch> *attribute = m_last_attribute; attribute; attribute = attribute->m_prev_attribute)
                if ((name.empty() || attribute->name() == name) && (xmlns.empty() || attribute->xmlns() == xmlns))
                    return attribute;
            return nullptr;
        }

        ///////////////////////////////////////////////////////////////////////////
        // Node modification

        //! Sets type of node.
        //! \param type Type of node to set.
        void type(node_type type) {
            m_type = type;
            dirty();
        }

        ///////////////////////////////////////////////////////////////////////////
        // Node manipulation

        //! Allocate a new element to be added as a child at this node.
        //! If an XMLNS is specified via the clarke notation syntax, then the prefix will match the parent element (if any),
        //! and any needed xmlns attributes will be added for you.
        //! Strings are assumed to remain in scope - you should document()->allocate_string() any that might not.
        //! \param name Name of the element, either string view, string, or clarke notation
    protected: // These are too easy to accidentally forget to append, prepend, or insert.
        optional_ptr<xml_node<Ch>> allocate_element(view_type const & name) {
            return document()->allocate_node(node_element, name);
        }
        optional_ptr<xml_node<Ch>> allocate_element(std::tuple<view_type,view_type> const & clark_name) {
            auto [xmlns, name] = clark_name;
            xml_node<Ch> * child;
            if (xmlns != this->xmlns()) {
                child = document()->allocate_node(node_element, name);
                child->append_attribute(document()->allocate_attribute("xmlns", xmlns));
            } else if (!this->prefix().empty()) {
                std::basic_string<Ch> pname = std::string(this->prefix()) + ':';
                pname += name;
                child = document()->allocate_node(node_element, document()->allocate_string(pname));
            } else {
                child = document()->allocate_node(node_element, name);
            }
            return child;
        }
        optional_ptr<xml_node<Ch>> allocate_element(view_type const & name, view_type const & value) {
            auto child = allocate_element(name);
            child->value(value);
            return child;
        }
        optional_ptr<xml_node<Ch>> allocate_element(std::tuple<view_type,view_type> const & clark_name, view_type const & value) {
            auto child = allocate_element(clark_name);
            child->value(value);
            return child;
        }
        optional_ptr<xml_node<Ch>> allocate_element(std::initializer_list<const Ch *> const & clark_name) {
            auto it = clark_name.begin();
            auto a = *it;
            auto b = *++it;
            return allocate_element({view_type(a), view_type(b)});
        }
        optional_ptr<xml_node<Ch>> allocate_element(std::initializer_list<const Ch *> const & clark_name, view_type const & value) {
            auto child = allocate_element(clark_name);
            if (!value.empty()) child->value(value);
            return child;
        }
    public:

        //! Prepends a new child node.
        //! The prepended child becomes the first child, and all existing children are moved one position back.
        //! \param child Node to prepend.
        optional_ptr<xml_node<Ch>> prepend_node(xml_node<Ch> *child)
        {
            assert(child && !child->parent() && child->type() != node_document);
            dirty();
            if (first_node())
            {
                child->m_next_sibling = m_first_node;
                m_first_node->m_prev_sibling = child;
            }
            else
            {
                child->m_next_sibling = 0;
                m_last_node = child;
            }
            m_first_node = child;
            child->m_parent = this;
            child->m_prev_sibling = 0;
            return child;
        }
        auto prepend_node(optional_ptr<xml_node<Ch>> ptr) {
            return prepend_node(ptr.get());
        }
        auto prepend_element(view_type const & v, view_type const & value = {}) {
            auto child = allocate_element(v, value);
            return prepend_node(child);
        }
        auto prepend_element(std::tuple<view_type, view_type> const & il, view_type const & value = {}) {
            auto child = allocate_element(il, value);
            return prepend_node(child);
        }
        auto prepend_element(std::initializer_list<const Ch *> const & il, view_type const & value = {}) {
            auto child = allocate_element(il, value);
            return prepend_node(child);
        }

        //! Appends a new child node.
        //! The appended child becomes the last child.
        //! \param child Node to append.
        optional_ptr<xml_node<Ch>> append_node(xml_node<Ch> *child)
        {
            assert(child && !child->parent() && child->type() != node_document);
            dirty();
            if (first_node())
            {
                child->m_prev_sibling = m_last_node;
                m_last_node->m_next_sibling = child;
            }
            else
            {
                child->m_prev_sibling = nullptr;
                m_first_node = child;
            }
            m_last_node = child;
            child->m_parent = this;
            child->m_next_sibling = nullptr;
            return child;
        }
        optional_ptr<xml_node<Ch>> append_node(optional_ptr<xml_node<Ch>> ptr) {
            return append_node(ptr.get());
        }
        auto append_element(view_type const & v, view_type const & value = {}) {
            auto child = allocate_element(v, value);
            return append_node(child);
        }
        auto append_element(std::tuple<view_type, view_type> const & il, view_type const & value = {}) {
            auto child = allocate_element(il, value);
            return append_node(child);
        }
        auto append_element(std::initializer_list<const Ch *> const & il, view_type const & value = {}) {
            auto child = allocate_element(il, value);
            return append_node(child);
        }

        //! Inserts a new child node at specified place inside the node.
        //! All children after and including the specified node are moved one position back.
        //! \param where Place where to insert the child, or 0 to insert at the back.
        //! \param child Node to insert.
        optional_ptr<xml_node<Ch>> insert_node(xml_node<Ch> *where, xml_node<Ch> *child)
        {
            assert(!where || where->parent() == this);
            assert(child && !child->parent() && child->type() != node_document);
            dirty();
            if (where == m_first_node)
                prepend_node(child);
            else if (!where)
                append_node(child);
            else
            {
                child->m_prev_sibling = where->m_prev_sibling;
                child->m_next_sibling = where;
                where->m_prev_sibling->m_next_sibling = child;
                where->m_prev_sibling = child;
                child->m_parent = this;
            }
            return child;
        }
        auto insert_node(optional_ptr<xml_node<Ch>> where, optional_ptr<xml_node<Ch>> ptr) {
            return insert_node(where.ptr(), ptr.ptr());
        }
        auto insert_element(optional_ptr<xml_node<Ch>> where, view_type const & v, view_type const & value = {}) {
            auto child = allocate_element(v, value);
            return insert_node(where, child);
        }
        auto insert_element(optional_ptr<xml_node<Ch>> where, std::tuple<view_type, view_type> const & il, view_type const & value = {}) {
            auto child = allocate_element(il, value);
            return insert_node(where, child);
        }
        auto insert_element(optional_ptr<xml_node<Ch>> where, std::initializer_list<const Ch *> const & il, view_type const & value = {}) {
            auto child = allocate_element(il, value);
            return insert_node(where, child);
        }

        //! Removes first child node.
        //! If node has no children, behaviour is undefined.
        //! Use first_node() to test if node has children.
        void remove_first_node()
        {
            assert(first_node());
            dirty();
            xml_node<Ch> *child = m_first_node;
            m_first_node = child->m_next_sibling;
            if (child->m_next_sibling)
                child->m_next_sibling->m_prev_sibling = nullptr;
            else
                m_last_node = nullptr;
            child->m_parent = nullptr;
        }

        //! Removes last child of the node.
        //! If node has no children, behaviour is undefined.
        //! Use first_node() to test if node has children.
        void remove_last_node()
        {
            assert(first_node());
            dirty();
            xml_node<Ch> *child = m_last_node;
            if (child->m_prev_sibling)
            {
                m_last_node = child->m_prev_sibling;
                child->m_prev_sibling->m_next_sibling = nullptr;
            }
            else
                m_first_node = nullptr;
            child->m_parent = nullptr;
        }

        //! Removes specified child from the node
        // \param where Pointer to child to be removed.
        void remove_node(optional_ptr<xml_node<Ch>> where)
        {
            assert(where->parent() == this);
            assert(first_node());
            dirty();
            if (where == m_first_node)
                remove_first_node();
            else if (where == m_last_node)
                remove_last_node();
            else
            {
                where->m_prev_sibling->m_next_sibling = where->m_next_sibling;
                where->m_next_sibling->m_prev_sibling = where->m_prev_sibling;
                where->m_parent = nullptr;
            }
        }

        //! Removes all child nodes (but not attributes).
        void remove_all_nodes()
        {
            if (!m_first_node) return;
            dirty();
            for (xml_node<Ch> *node = m_first_node; node; node = node->m_next_sibling) {
                node->m_parent = nullptr;
            }
            m_first_node = nullptr;
            m_last_node = nullptr;
        }

        //! Prepends a new attribute to the node.
        //! \param attribute Attribute to prepend.
        void prepend_attribute(xml_attribute<Ch> *attribute)
        {
            assert(attribute && !attribute->parent());
            dirty_parent();
            if (first_attribute())
            {
                attribute->m_next_attribute = m_first_attribute;
                m_first_attribute->m_prev_attribute = attribute;
            }
            else
            {
                attribute->m_next_attribute = nullptr;
                m_last_attribute = attribute;
            }
            m_first_attribute = attribute;
            attribute->m_parent = this;
            attribute->m_prev_attribute = nullptr;
        }

        //! Appends a new attribute to the node.
        //! \param attribute Attribute to append.
        void append_attribute(xml_attribute<Ch> *attribute)
        {
            assert(attribute && !attribute->parent());
            dirty_parent();
            if (first_attribute())
            {
                attribute->m_prev_attribute = m_last_attribute;
                m_last_attribute->m_next_attribute = attribute;
            }
            else
            {
                attribute->m_prev_attribute = nullptr;
                m_first_attribute = attribute;
            }
            m_last_attribute = attribute;
            attribute->m_parent = this;
            attribute->m_next_attribute = nullptr;
        }

        //! Inserts a new attribute at specified place inside the node.
        //! All attributes after and including the specified attribute are moved one position back.
        //! \param where Place where to insert the attribute, or 0 to insert at the back.
        //! \param attribute Attribute to insert.
        void insert_attribute(xml_attribute<Ch> *where, xml_attribute<Ch> *attribute)
        {
            assert(!where || where->parent() == this);
            assert(attribute && !attribute->parent());
            dirty_parent();
            if (where == m_first_attribute)
                prepend_attribute(attribute);
            else if (!where)
                append_attribute(attribute);
            else
            {
                attribute->m_prev_attribute = where->m_prev_attribute;
                attribute->m_next_attribute = where;
                where->m_prev_attribute->m_next_attribute = attribute;
                where->m_prev_attribute = attribute;
                attribute->m_parent = this;
            }
        }

        //! Removes first attribute of the node.
        //! If node has no attributes, behaviour is undefined.
        //! Use first_attribute() to test if node has attributes.
        void remove_first_attribute()
        {
            assert(first_attribute());
            dirty_parent();
            xml_attribute<Ch> *attribute = m_first_attribute;
            if (attribute->m_next_attribute)
            {
                attribute->m_next_attribute->m_prev_attribute = 0;
            }
            else
                m_last_attribute = nullptr;
            attribute->m_parent = nullptr;
            m_first_attribute = attribute->m_next_attribute;
        }

        //! Removes last attribute of the node.
        //! If node has no attributes, behaviour is undefined.
        //! Use first_attribute() to test if node has attributes.
        void remove_last_attribute()
        {
            assert(first_attribute());
            dirty_parent();
            xml_attribute<Ch> *attribute = m_last_attribute;
            if (attribute->m_prev_attribute)
            {
                attribute->m_prev_attribute->m_next_attribute = 0;
                m_last_attribute = attribute->m_prev_attribute;
            }
            else
                m_first_attribute = nullptr;
            attribute->m_parent = nullptr;
        }

        //! Removes specified attribute from node.
        //! \param where Pointer to attribute to be removed.
        void remove_attribute(optional_ptr<xml_attribute<Ch>> where)
        {
            assert(first_attribute() && where->parent() == this);
            dirty_parent();
            if (where == m_first_attribute)
                remove_first_attribute();
            else if (where == m_last_attribute)
                remove_last_attribute();
            else
            {
                where->m_prev_attribute->m_next_attribute = where->m_next_attribute;
                where->m_next_attribute->m_prev_attribute = where->m_prev_attribute;
                where->m_parent = nullptr;
            }
        }

        //! Removes all attributes of node.
        void remove_all_attributes()
        {
            if (!m_first_attribute) return;
            dirty_parent();
            for (xml_attribute<Ch> *attribute = m_first_attribute; attribute; attribute = attribute->m_next_attribute) {
                attribute->m_parent = nullptr;
            }
            m_first_attribute = nullptr;
        }

        void validate() const
        {
            this->xmlns();
            for (auto child = this->first_node();
                 child;
                 child = child->next_sibling()) {
                child->validate();
            }
            for (auto attribute = first_attribute();
                 attribute;
                 attribute = attribute->m_next_attribute) {
                attribute->xmlns();
                for (auto otherattr = first_attribute();
                     otherattr != attribute;
                     otherattr = otherattr->m_next_attribute) {
                    if (attribute->name() == otherattr->name()) {
                        throw duplicate_attribute("Attribute doubled");
                    }
                    if ((attribute->local_name() == otherattr->local_name())
                        && (attribute->xmlns() == otherattr->xmlns()))
                        throw duplicate_attribute("Attribute XMLNS doubled");
                }
            }
        }

    private:

        ///////////////////////////////////////////////////////////////////////////
        // Restrictions

        // No copying
        xml_node(const xml_node &) = delete;
        void operator =(const xml_node &) = delete;

        ///////////////////////////////////////////////////////////////////////////
        // Data members

        // Note that some of the pointers below have UNDEFINED values if certain other pointers are 0.
        // This is required for maximum performance, as it allows the parser to omit initialization of
        // unneded/redundant values.
        //
        // The rules are as follows:
        // 1. first_node and first_attribute contain valid pointers, or 0 if node has no children/attributes respectively
        // 2. last_node and last_attribute are valid only if node has at least one child/attribute respectively, otherwise they contain garbage
        // 3. prev_sibling and next_sibling are valid only if node has a parent, otherwise they contain garbage

        view_type m_prefix;
        mutable std::optional<view_type> m_xmlns; // Cache
        node_type m_type;                       // Type of node; always valid
        xml_node<Ch> *m_first_node = nullptr;             // Pointer to first child node, or 0 if none; always valid
        xml_node<Ch> *m_last_node = nullptr;              // Pointer to last child node, or 0 if none; this value is only valid if m_first_node is non-zero
        xml_attribute<Ch> *m_first_attribute = nullptr;   // Pointer to first attribute of node, or 0 if none; always valid
        xml_attribute<Ch> *m_last_attribute = nullptr;    // Pointer to last attribute of node, or 0 if none; this value is only valid if m_first_attribute is non-zero
        xml_node<Ch> *m_prev_sibling = nullptr;           // Pointer to previous sibling of node, or 0 if none; this value is only valid if m_parent is non-zero
        xml_node<Ch> *m_next_sibling = nullptr;           // Pointer to next sibling of node, or 0 if none; this value is only valid if m_parent is non-zero
        view_type m_contents;                   // Pointer to original contents in buffer.
        bool m_clean = false; // Unchanged since parsing (ie, contents are good).
        mutable std::optional<view_type> m_value;
    };

    ///////////////////////////////////////////////////////////////////////////
    // XML document

    //! This class represents root of the DOM hierarchy.
    //! It is also an xml_node and a memory_pool through public inheritance.
    //! Use parse() function to build a DOM tree from a zero-terminated XML text string.
    //! parse() function allocates memory for nodes and attributes by using functions of xml_document,
    //! which are inherited from memory_pool.
    //! To access root node of the document, use the document itself, as if it was an xml_node.
    //! \param Ch Character type to use.
    template<class Ch = char>
    class xml_document: public xml_node<Ch>, public memory_pool<Ch>
    {
    public:
        using view_type = std::basic_string_view<Ch>;
        using ptr = optional_ptr<xml_document<Ch>>;

        //! Constructs empty XML document
        xml_document()
            : xml_node<Ch>(node_document)
        {
        }

        //! Parses zero-terminated XML string according to given flags.
        //! Passed string will be modified by the parser, unless rapidxml::parse_non_destructive flag is used.
        //! The string must persist for the lifetime of the document.
        //! In case of error, rapidxml::parse_error exception will be thrown.
        //! <br><br>
        //! If you want to parse contents of a file, you must first load the file into the memory, and pass pointer to its beginning.
        //! Make sure that data is zero-terminated.
        //! <br><br>
        //! Document can be parsed into multiple times.
        //! Each new call to parse removes previous nodes and attributes (if any), but does not clear memory pool.
        //! \param text XML data to parse; pointer is non-const to denote fact that this data may be modified by the parser.
        template<int Flags>
        auto parse(const Ch * text, xml_document<Ch> * parent = nullptr) {
            return this->parse_low<Flags>(text, parent);
        }

        template<int Flags>
        auto parse(std::basic_string<Ch> const & str, xml_document<Ch> * parent = nullptr) {
            return this->parse_low<Flags>(str.c_str(), parent);
        }

        template<int Flags, typename C>
        requires std::is_same_v<Ch, typename C::value_type>
        auto parse(C const & container, xml_document<Ch> * parent = nullptr) {
            return this->parse_low<Flags>(buffer_ptr<C>(container), parent);
        }

        template<int Flags, typename T>
        T parse_low(T text, xml_document<Ch> * parent) {
            this->m_parse_flags = Flags;

            // Remove current contents
            this->remove_all_nodes();
            this->remove_all_attributes();
            this->m_parent = parent ? parent->first_node().get() : nullptr;

            // Parse BOM, if any
            parse_bom<Flags>(text);

            // Parse children
            while (true)
            {
                // Skip whitespace before node
                skip<whitespace_pred, Flags>(text);
                if (*text == 0)
                    break;

                // Parse and append new child
                if (*text == Ch('<'))
                {
                    ++text;     // Skip '<'
                    if (xml_node<Ch> *node = parse_node<Flags>(text)) {
                        this->append_node(node);
                        if (Flags & (parse_open_only|parse_parse_one) && node->type() == node_element) {
                            break;
                        }
                    }
                }
                else
                    FLXML_PARSE_ERROR("expected <", text);
            }
            if (!this->first_node()) FLXML_PARSE_ERROR("no root element", text);
            return text;
        }

        //! Clears the document by deleting all nodes and clearing the memory pool.
        //! All nodes owned by document pool are destroyed.
        void clear()
        {
            this->remove_all_nodes();
            this->remove_all_attributes();
            memory_pool<Ch>::clear();
        }

        template<int Flags>
        view_type decode_data_value_low(view_type const & v) {
            buffer_ptr first{v};
            if (Flags & parse_normalize_whitespace) {
                skip<text_pure_with_ws_pred,0>(first);
            } else {
                skip<text_pure_no_ws_pred,0>(first);
            }
            if (!*first) return v;
            auto buf = this->allocate_string(v);
            auto * start = buf.data();
            buffer_ptr tmp{buf};
            auto end = (Flags & parse_normalize_whitespace) ?
                    skip_and_expand_character_refs<text_pred,text_pure_with_ws_pred,Flags>(tmp) :
                    skip_and_expand_character_refs<text_pred,text_pure_no_ws_pred,Flags>(tmp);
            // Trim trailing whitespace if flag is set; leading was already trimmed by whitespace skip after >
            if (Flags & parse_trim_whitespace)
            {
                if (Flags & parse_normalize_whitespace)
                {
                    // Whitespace is already condensed to single space characters by skipping function, so just trim 1 char off the end
                    if (*(end - 1) == Ch(' '))
                        --end;
                }
                else
                {
                    // Backup until non-whitespace character is found
                    while (whitespace_pred::test(*(end - 1)))
                        --end;
                }
            }

            return {start, end};
        }

        template<Ch Q>
        view_type decode_attr_value_low(view_type const & v) {
            buffer_ptr first{v};
            skip<attribute_value_pure_pred<Q>,0>(first);
            if (!*first || *first == Q) return v;
            auto buf = this->allocate_string(v);
            const Ch * start = buf.data();
            buffer_ptr tmp{buf};
            const Ch * end = skip_and_expand_character_refs<attribute_value_pred<Q>,attribute_value_pure_pred<Q>,0>(tmp);
            return {start, end};
        }

        view_type decode_attr_value(const xml_attribute<Ch> * attr) {
            if (attr->quote() == Ch('"')) {
                return decode_attr_value_low<'"'>(attr->value_raw());
            } else if (attr->quote() == Ch('\'')){
                return decode_attr_value_low<'\''>(attr->value_raw());
            } else {
                return attr->value_raw();
            }
        }

        view_type decode_data_value(const xml_node<Ch> * node) {
            if (node->value_raw().empty()) return node->value_raw();
            if (m_parse_flags & parse_normalize_whitespace) {
                if (m_parse_flags & parse_trim_whitespace) {
                    const int Flags = parse_normalize_whitespace | parse_trim_whitespace;
                    return decode_data_value_low<Flags>(node->value_raw());
                } else {
                    const int Flags = parse_normalize_whitespace;
                    return decode_data_value_low<Flags>(node->value_raw());
                }
            } else {
                if (m_parse_flags & parse_trim_whitespace) {
                    const int Flags = parse_trim_whitespace;
                    return decode_data_value_low<Flags>(node->value_raw());
                } else {
                    const int Flags = 0;
                    return decode_data_value_low<Flags>(node->value_raw());
                }
            }
        }

        void validate() const
        {
            for (auto child = this->first_node();
                 child;
                 child = child->next_sibling()) {
                child->validate();
            }
        }

#ifndef RAPIDXML_TESTING
    private:
#endif

        ///////////////////////////////////////////////////////////////////////
        // Internal character utility functions

        // Detect whitespace character
        struct whitespace_pred
        {
            static unsigned char test(Ch ch)
            {
                return internal::lookup_tables::lookup_whitespace[static_cast<unsigned char>(ch)];
            }
        };

        // Detect node name character
        struct node_name_pred
        {
            static unsigned char test(Ch ch)
            {
                return internal::lookup_tables::lookup_node_name[static_cast<unsigned char>(ch)];
            }
        };

        // Detect element name character
        struct element_name_pred
        {
            static unsigned char test(Ch ch)
            {
                return internal::lookup_tables::lookup_element_name[static_cast<unsigned char>(ch)];
            }
        };

        // Detect attribute name character
        struct attribute_name_pred
        {
            static unsigned char test(Ch ch)
            {
                return internal::lookup_tables::lookup_attribute_name[static_cast<unsigned char>(ch)];
            }
        };

        // Detect text character (PCDATA)
        struct text_pred
        {
            static unsigned char test(Ch ch)
            {
                return internal::lookup_tables::lookup_text[static_cast<unsigned char>(ch)];
            }
        };

        // Detect text character (PCDATA) that does not require processing
        struct text_pure_no_ws_pred
        {
            static unsigned char test(Ch ch)
            {
                return internal::lookup_tables::lookup_text_pure_no_ws[static_cast<unsigned char>(ch)];
            }
        };

        // Detect text character (PCDATA) that does not require processing
        struct text_pure_with_ws_pred
        {
            static unsigned char test(Ch ch)
            {
                return internal::lookup_tables::lookup_text_pure_with_ws[static_cast<unsigned char>(ch)];
            }
        };

        // Detect attribute value character
        template<Ch Quote>
        struct attribute_value_pred
        {
            static unsigned char test(Ch ch)
            {
                if (Quote == Ch('\''))
                    return internal::lookup_tables::lookup_attribute_data_1[static_cast<unsigned char>(ch)];
                if (Quote == Ch('\"'))
                    return internal::lookup_tables::lookup_attribute_data_2[static_cast<unsigned char>(ch)];
                return 0;       // Should never be executed, to avoid warnings on Comeau
            }
        };

        // Detect attribute value character
        template<Ch Quote>
        struct attribute_value_pure_pred
        {
            static unsigned char test(Ch ch)
            {
                if (Quote == Ch('\''))
                    return internal::lookup_tables::lookup_attribute_data_1_pure[static_cast<unsigned char>(ch)];
                if (Quote == Ch('\"'))
                    return internal::lookup_tables::lookup_attribute_data_2_pure[static_cast<unsigned char>(ch)];
                return 0;       // Should never be executed, to avoid warnings on Comeau
            }
        };

        // Insert coded character, using UTF8 or 8-bit ASCII
        template<int Flags>
        static void insert_coded_character(Ch *&text, unsigned long code)
        {
            if (Flags & parse_no_utf8)
            {
                // Insert 8-bit ASCII character
                // Todo: possibly verify that code is less than 256 and use replacement char otherwise?
                text[0] = static_cast<unsigned char>(code);
                text += 1;
            }
            else
            {
                // Insert UTF8 sequence
                if (code < 0x80)    // 1 byte sequence
                {
	                text[0] = static_cast<unsigned char>(code);
                    text += 1;
                }
                else if (code < 0x800)  // 2 byte sequence
                {
	                text[1] = static_cast<unsigned char>((code | 0x80) & 0xBF); code >>= 6;
	                text[0] = static_cast<unsigned char>(code | 0xC0);
                    text += 2;
                }
	            else if (code < 0x10000)    // 3 byte sequence
                {
	                text[2] = static_cast<unsigned char>((code | 0x80) & 0xBF); code >>= 6;
	                text[1] = static_cast<unsigned char>((code | 0x80) & 0xBF); code >>= 6;
	                text[0] = static_cast<unsigned char>(code | 0xE0);
                    text += 3;
                }
	            else if (code < 0x110000)   // 4 byte sequence
                {
	                text[3] = static_cast<unsigned char>((code | 0x80) & 0xBF); code >>= 6;
	                text[2] = static_cast<unsigned char>((code | 0x80) & 0xBF); code >>= 6;
	                text[1] = static_cast<unsigned char>((code | 0x80) & 0xBF); code >>= 6;
	                text[0] = static_cast<unsigned char>(code | 0xF0);
                    text += 4;
                }
                else    // Invalid, only codes up to 0x10FFFF are allowed in Unicode
                {
                    FLXML_PARSE_ERROR("invalid numeric character entity", text);
                }
            }
        }

        // Skip characters until predicate evaluates to true
        template<class StopPred, int Flags,  typename Chp>
        static void skip(Chp & b)
        {
            while (StopPred::test(*b))
                ++b;
        }

        // Skip characters until predicate evaluates to true while doing the following:
        // - replacing XML character entity references with proper characters (&apos; &amp; &quot; &lt; &gt; &#...;)
        // - condensing whitespace sequences to single space character
        template<class StopPred, class StopPredPure, int Flags, typename Chp>
        static const Ch *skip_and_expand_character_refs(Chp text)
        {
            // If entity translation, whitespace condense and whitespace trimming is disabled, use plain skip
            if (Flags & parse_no_entity_translation &&
                !(Flags & parse_normalize_whitespace) &&
                !(Flags & parse_trim_whitespace))
            {
                skip<StopPred, Flags>(text);
                return &*text;
            }

            // Use simple skip until first modification is detected
            skip<StopPredPure, Flags>(text);

            // Use translation skip
            Chp src = text;
            Ch * dest = const_cast<Ch *>(&*src);
            while (StopPred::test(*src))
            {
                // If entity translation is enabled
                if (!(Flags & parse_no_entity_translation))
                {
                    // Test if replacement is needed
                    if (src[0] == Ch('&'))
                    {
                        switch (src[1])
                        {

                        // &amp; &apos;
                        case Ch('a'):
                            if (src[2] == Ch('m') && src[3] == Ch('p') && src[4] == Ch(';'))
                            {
                                *dest = Ch('&');
                                ++dest;
                                src += 5;
                                continue;
                            }
                            if (src[2] == Ch('p') && src[3] == Ch('o') && src[4] == Ch('s') && src[5] == Ch(';'))
                            {
                                *dest = Ch('\'');
                                ++dest;
                                src += 6;
                                continue;
                            }
                            break;

                        // &quot;
                        case Ch('q'):
                            if (src[2] == Ch('u') && src[3] == Ch('o') && src[4] == Ch('t') && src[5] == Ch(';'))
                            {
                                *dest = Ch('"');
                                ++dest;
                                src += 6;
                                continue;
                            }
                            break;

                        // &gt;
                        case Ch('g'):
                            if (src[2] == Ch('t') && src[3] == Ch(';'))
                            {
                                *dest = Ch('>');
                                ++dest;
                                src += 4;
                                continue;
                            }
                            break;

                        // &lt;
                        case Ch('l'):
                            if (src[2] == Ch('t') && src[3] == Ch(';'))
                            {
                                *dest = Ch('<');
                                ++dest;
                                src += 4;
                                continue;
                            }
                            break;

                        // &#...; - assumes ASCII
                        case Ch('#'):
                            if (src[2] == Ch('x'))
                            {
                                unsigned long code = 0;
                                src += 3;   // Skip &#x
                                while (true)
                                {
                                    unsigned char digit = internal::lookup_tables::lookup_digits[static_cast<unsigned char>(*src)];
                                    if (digit == 0xFF)
                                        break;
                                    code = code * 16 + digit;
                                    ++src;
                                }
                                insert_coded_character<Flags>(dest, code);    // Put character in output
                            }
                            else
                            {
                                unsigned long code = 0;
                                src += 2;   // Skip &#
                                while (true)
                                {
                                    unsigned char digit = internal::lookup_tables::lookup_digits[static_cast<unsigned char>(*src)];
                                    if (digit == 0xFF)
                                        break;
                                    code = code * 10 + digit;
                                    ++src;
                                }
                                insert_coded_character<Flags>(dest, code);    // Put character in output
                            }
                            if (*src == Ch(';'))
                                ++src;
                            else
                                FLXML_PARSE_ERROR("expected ;", src);
                            continue;

                        // Something else
                        default:
                            // Ignore, just copy '&' verbatim
                            break;

                        }
                    }
                }

                // If whitespace condensing is enabled
                if (Flags & parse_normalize_whitespace && whitespace_pred::test(*src)) {
                    *dest = Ch(' '); ++dest;    // Put single space in dest
                    ++src;                      // Skip first whitespace char
                    // Skip remaining whitespace chars
                    while (whitespace_pred::test(*src))
                        ++src;
                    continue;
                }

                // No replacement, only copy character
                *dest++ = *src++;

            }

            // Return new end
            return dest;
        }

        ///////////////////////////////////////////////////////////////////////
        // Internal parsing functions

        // Parse BOM, if any
        template<int Flags, typename Chp>
        void parse_bom(Chp &texta)
        {
            Chp text = texta;
            // UTF-8?
            if (static_cast<unsigned char>(*text++) == 0xEF &&
                static_cast<unsigned char>(*text++) == 0xBB &&
                static_cast<unsigned char>(*text++) == 0xBF)
            {
                texta = text;      // Skup utf-8 bom
            }
        }

        // Parse XML declaration (<?xml...)
        template<int Flags, typename Chp>
        xml_node<Ch> *parse_xml_declaration(Chp &text)
        {
            // If parsing of declaration is disabled
            if (!(Flags & parse_declaration_node))
            {
                // Skip until end of declaration
                while (text[0] != Ch('?') || text[1] != Ch('>'))
                {
                    if (!text[0]) FLXML_PARSE_ERROR("unexpected end of data", text);
                    ++text;
                }
                text += 2;    // Skip '?>'
                return 0;
            }

            // Create declaration
            xml_node<Ch> *declaration = this->allocate_node(node_declaration);

            // Skip whitespace before attributes or ?>
            skip<whitespace_pred, Flags>(text);

            // Parse declaration attributes
            parse_node_attributes<Flags>(text, declaration);

            // Skip ?>
            if (text[0] != Ch('?') || text[1] != Ch('>')) FLXML_PARSE_ERROR("expected ?>", text);
            text += 2;

            return declaration;
        }

        // Parse XML comment (<!--...)
        template<int Flags, typename Chp>
        xml_node<Ch> *parse_comment(Chp &text)
        {
            // If parsing of comments is disabled
            if (!(Flags & parse_comment_nodes))
            {
                // Skip until end of comment
                while (text[0] != Ch('-') || text[1] != Ch('-') || text[2] != Ch('>'))
                {
                    if (!text[0]) FLXML_PARSE_ERROR("unexpected end of data", text);
                    ++text;
                }
                text += 3;     // Skip '-->'
                return 0;      // Do not produce comment node
            }

            // Remember value start
            Chp value = text;

            // Skip until end of comment
            while (text[0] != Ch('-') || text[1] != Ch('-') || text[2] != Ch('>'))
            {
                if (!text[0]) FLXML_PARSE_ERROR("unexpected end of data", text);
                ++text;
            }

            // Create comment node
            xml_node<Ch> *comment = this->allocate_node(node_comment);
            comment->value({value, text});

            text += 3;     // Skip '-->'
            return comment;
        }

        // Parse DOCTYPE
        template<int Flags, typename Chp>
        xml_node<Ch> *parse_doctype(Chp &text)
        {
            // Remember value start
            Chp value = text;

            // Skip to >
            while (*text != Ch('>'))
            {
                // Determine character type
                switch (*text)
                {

                // If '[' encountered, scan for matching ending ']' using naive algorithm with depth
                // This works for all W3C test files except for 2 most wicked
                case Ch('['):
                {
                    ++text;     // Skip '['
                    int depth = 1;
                    while (depth > 0)
                    {
                        switch (*text)
                        {
                            case Ch('['): ++depth; break;
                            case Ch(']'): --depth; break;
                            case 0: FLXML_PARSE_ERROR("unexpected end of data", text);
                            default: break;
                        }
                        ++text;
                    }
                    break;
                }

                // Error on end of text
                case Ch('\0'):
                    FLXML_PARSE_ERROR("unexpected end of data", text);

                // Other character, skip it
                default:
                    ++text;

                }
            }

            // If DOCTYPE nodes enabled
            if (Flags & parse_doctype_node)
            {
                // Create a new doctype node
                xml_node<Ch> *doctype = this->allocate_node(node_doctype);
                doctype->value({value, text});

                text += 1;      // skip '>'
                return doctype;
            }
            else
            {
                text += 1;      // skip '>'
                return 0;
            }

        }

        // Parse PI
        template<int Flags, typename Chp>
        xml_node<Ch> *parse_pi(Chp &text)
        {
            // If creation of PI nodes is enabled
            if (Flags & parse_pi_nodes)
            {
                // Create pi node
                xml_node<Ch> *pi = this->allocate_node(node_pi);

                // Extract PI target name
                Chp name = text;
                skip<node_name_pred, Flags>(text);
                if (text == name) FLXML_PARSE_ERROR("expected PI target", text);
                pi->name({name, text});

                // Skip whitespace between pi target and pi
                skip<whitespace_pred, Flags>(text);

                // Remember start of pi
                Chp value = text;

                // Skip to '?>'
                while (text[0] != Ch('?') || text[1] != Ch('>'))
                {
                    if (*text == Ch('\0'))
                        FLXML_PARSE_ERROR("unexpected end of data", text);
                    ++text;
                }

                // Set pi value (verbatim, no entity expansion or whitespace normalization)
                pi->value({value, text});

                text += 2;                          // Skip '?>'
                return pi;
            }
            else
            {
                // Skip to '?>'
                while (text[0] != Ch('?') || text[1] != Ch('>'))
                {
                    if (*text == Ch('\0'))
                        FLXML_PARSE_ERROR("unexpected end of data", text);
                    ++text;
                }
                text += 2;    // Skip '?>'
                return 0;
            }
        }

        // Parse and append data
        // Return character that ends data.
        // This is necessary because this character might have been overwritten by a terminating 0
        template<int Flags, typename Chp>
        Ch parse_and_append_data(xml_node<Ch> *node, Chp &text, Chp contents_start)
        {
            // Backup to contents start if whitespace trimming is disabled
            if (!(Flags & parse_trim_whitespace))
                text = contents_start;

            // Skip until end of data. We should check if the contents will need decoding.
            Chp value = text;
            bool encoded = false;
            skip<text_pure_no_ws_pred,0>(text);
            if (text_pred::test(*text)) {
                encoded = true;
                skip<text_pred,0>(text);
            }

            // If characters are still left between end and value (this test is only necessary if normalization is enabled)
            // Create new data node
            if (!(Flags & parse_no_data_nodes))
            {
                xml_node<Ch> *data = this->allocate_node(node_data);
                data->value_raw({value, text});
                if (!encoded) data->value(data->value_raw());
                node->append_node(data);
            }

            // Add data to parent node if no data exists yet
            if (!(Flags & parse_no_element_values)) {
                if (node->value_raw().empty()) {
                    node->value_raw({value, text});
                    if (!encoded) node->value(node->value_raw());
                }
            }

            // Return character that ends data
            return *text;
        }

        // Parse CDATA
        template<int Flags, typename Chp>
        xml_node<Ch> *parse_cdata(Chp &text)
        {
            // If CDATA is disabled
            if (Flags & parse_no_data_nodes)
            {
                // Skip until end of cdata
                while (text[0] != Ch(']') || text[1] != Ch(']') || text[2] != Ch('>'))
                {
                    if (!text[0])
                        FLXML_PARSE_ERROR("unexpected end of data", text);
                    ++text;
                }
                text += 3;      // Skip ]]>
                return 0;       // Do not produce CDATA node
            }

            // Skip until end of cdata
            Chp value = text;
            while (text[0] != Ch(']') || text[1] != Ch(']') || text[2] != Ch('>'))
            {
                if (!text[0])
                    FLXML_PARSE_ERROR("unexpected end of data", text);
                ++text;
            }

            // Create new cdata node
            xml_node<Ch> *cdata = this->allocate_node(node_cdata);
            cdata->value({value, text});

            text += 3;      // Skip ]]>
            return cdata;
        }

        // Parse element node
        template<int Flags, typename Chp>
        xml_node<Ch> *parse_element(Chp &text)
        {
            // Create element node
            xml_node<Ch> *element = this->allocate_node(node_element);

            // Extract element name
            Chp prefix = text;
            view_type qname;
            skip<element_name_pred, Flags>(text);
            if (text == prefix)
                FLXML_PARSE_ERROR("expected element name or prefix", text);
            if (*text == Ch(':')) {
                element->prefix({prefix, text});
                ++text;
                Chp name = text;
                skip<node_name_pred, Flags>(text);
                if (text == name)
                    FLXML_PARSE_ERROR("expected element local name", text);
                element->name({name, text});
            } else {
                element->name({prefix, text});
            }
            qname = {prefix, text};

            // Skip whitespace between element name and attributes or >
            skip<whitespace_pred, Flags>(text);

            // Parse attributes, if any
            parse_node_attributes<Flags>(text, element);
            // Once we have all the attributes, we should be able to fully validate:
            if (Flags & parse_validate_xmlns) this->validate();

            // Determine ending type
            if (*text == Ch('>'))
            {
                Chp contents = ++text;
                Chp contents_end = contents;
                if (!(Flags & parse_open_only))
                    contents_end = parse_node_contents<Flags>(text, element, qname);
                if (contents != contents_end) element->contents({contents, contents_end});
            }
            else if (*text == Ch('/'))
            {
                ++text;
                if (*text != Ch('>'))
                    FLXML_PARSE_ERROR("expected >", text);
                ++text;
                if (Flags & parse_open_only)
                    FLXML_PARSE_ERROR("open_only, but closed", text);
            }
            else
                FLXML_PARSE_ERROR("expected >", text);

            // Return parsed element
            return element;
        }

        // Determine node type, and parse it
        template<int Flags, typename Chp>
        xml_node<Ch> *parse_node(Chp &text)
        {
            // Parse proper node type
            switch (text[0])
            {

            // <...
            default:
                // Parse and append element node
                return parse_element<Flags>(text);

            // <?...
            case Ch('?'):
                ++text;     // Skip ?
                if ((text[0] == Ch('x') || text[0] == Ch('X')) &&
                    (text[1] == Ch('m') || text[1] == Ch('M')) &&
                    (text[2] == Ch('l') || text[2] == Ch('L')) &&
                    whitespace_pred::test(text[3]))
                {
                    // '<?xml ' - xml declaration
                    text += 4;      // Skip 'xml '
                    return parse_xml_declaration<Flags>(text);
                }
                else
                {
                    // Parse PI
                    return parse_pi<Flags>(text);
                }

            // <!...
            case Ch('!'):

                // Parse proper subset of <! node
                switch (text[1])
                {

                // <!-
                case Ch('-'):
                    if (text[2] == Ch('-'))
                    {
                        // '<!--' - xml comment
                        text += 3;     // Skip '!--'
                        return parse_comment<Flags>(text);
                    }
                    break;

                // <![
                case Ch('['):
                    if (text[2] == Ch('C') && text[3] == Ch('D') && text[4] == Ch('A') &&
                        text[5] == Ch('T') && text[6] == Ch('A') && text[7] == Ch('['))
                    {
                        // '<![CDATA[' - cdata
                        text += 8;     // Skip '![CDATA['
                        return parse_cdata<Flags>(text);
                    }
                    break;

                // <!D
                case Ch('D'):
                    if (text[2] == Ch('O') && text[3] == Ch('C') && text[4] == Ch('T') &&
                        text[5] == Ch('Y') && text[6] == Ch('P') && text[7] == Ch('E') &&
                        whitespace_pred::test(text[8]))
                    {
                        // '<!DOCTYPE ' - doctype
                        text += 9;      // skip '!DOCTYPE '
                        return parse_doctype<Flags>(text);
                    }

                    default:
                        break;
                }   // switch

                // Attempt to skip other, unrecognized node types starting with <!
                ++text;     // Skip !
                while (*text != Ch('>'))
                {
                    if (*text == 0)
                        FLXML_PARSE_ERROR("unexpected end of data", text);
                    ++text;
                }
                ++text;     // Skip '>'
                return 0;   // No node recognized

            }
        }

        // Parse contents of the node - children, data etc.
        // Return end pointer.
        template<int Flags, typename Chp>
        Chp parse_node_contents(Chp &text, xml_node<Ch> *node, view_type const & qname)
        {
            Chp retval;
            // For all children and text
            while (true)
            {
                // Skip whitespace between > and node contents
                Chp contents_start = text;      // Store start of node contents before whitespace is skipped
                skip<whitespace_pred, Flags>(text);
                Ch next_char = *text;

            // After data nodes, instead of continuing the loop, control jumps here.
            // This is because zero termination inside parse_and_append_data() function
            // would wreak havoc with the above code.
            // Also, skipping whitespace after data nodes is unnecessary.
            after_data_node:

                // Determine what comes next: node closing, child node, data node, or 0?
                switch (next_char)
                {

                // Node closing or child node
                case Ch('<'):
                    if (text[1] == Ch('/'))
                    {
                        // Node closing
                        retval = text;
                        text += 2;      // Skip '</'
                        if (Flags & parse_validate_closing_tags)
                        {
                            // Skip and validate closing tag name
                            Chp closing_name = text;
                            skip<node_name_pred, Flags>(text);
                            if (qname != view_type{closing_name, text})
                                FLXML_PARSE_ERROR("invalid closing tag name", text);
                        }
                        else
                        {
                            // No validation, just skip name
                            skip<node_name_pred, Flags>(text);
                        }
                        // Skip remaining whitespace after node name
                        skip<whitespace_pred, Flags>(text);
                        if (*text != Ch('>'))
                            FLXML_PARSE_ERROR("expected >", text);
                        ++text;     // Skip '>'
                        if (Flags & parse_open_only)
                            FLXML_PARSE_ERROR("Unclosed element actually closed.", text);
                        return retval;     // Node closed, finished parsing contents
                    }
                    else
                    {
                        // Child node
                        ++text;     // Skip '<'
                        if (xml_node<Ch> *child = parse_node<Flags & ~parse_open_only>(text))
                            node->append_node(child);
                    }
                    break;

                // End of data - error unless we expected this.
                case Ch('\0'):
                    if (Flags & parse_open_only) {
                        return Chp();
                    } else {
                        FLXML_PARSE_ERROR("unexpected end of data", text);
                    }

                // Data node
                default:
                    next_char = parse_and_append_data<Flags>(node, text, contents_start);
                    goto after_data_node;   // Bypass regular processing after data nodes

                }
            }
        }

        // Parse XML attributes of the node
        template<int Flags, typename Chp>
        void parse_node_attributes(Chp &text, xml_node<Ch> *node)
        {
            // For all attributes
            while (attribute_name_pred::test(*text))
            {
                // Extract attribute name
                Chp name = text;
                ++text;     // Skip first character of attribute name
                skip<attribute_name_pred, Flags>(text);
                if (text == name)
                    FLXML_PARSE_ERROR("expected attribute name", name);

                // Create new attribute
                xml_attribute<Ch> *attribute = this->allocate_attribute(view_type{name, text});
                node->append_attribute(attribute);

                // Skip whitespace after attribute name
                skip<whitespace_pred, Flags>(text);

                // Skip =
                if (*text != Ch('='))
                    FLXML_PARSE_ERROR("expected =", text);
                ++text;

                // Skip whitespace after =
                skip<whitespace_pred, Flags>(text);

                // Skip quote and remember if it was ' or "
                Ch quote = *text;
                if (quote != Ch('\'') && quote != Ch('"'))
                    FLXML_PARSE_ERROR("expected ' or \"", text);
                attribute->quote(quote);
                ++text;

                // Extract attribute value and expand char refs in it
                Chp value = text;
                Chp end;
                const int AttFlags = Flags & ~parse_normalize_whitespace;   // No whitespace normalization in attributes
                if (quote == Ch('\''))
                    skip<attribute_value_pred<Ch('\'')>, AttFlags>(text);
                else
                    skip<attribute_value_pred<Ch('"')>, AttFlags>(text);
                end = text;

                // Set attribute value
                attribute->value_raw({value, end});

                // Make sure that end quote is present
                if (*text != quote)
                    FLXML_PARSE_ERROR("expected ' or \"", text);
                ++text;     // Skip quote

                // Skip whitespace after attribute value
                skip<whitespace_pred, Flags>(text);
            }
        }
    private:
        int m_parse_flags = 0;
    };


}

// Also include this now.
#include <flxml/iterators.h>

// Undefine internal macros
#undef FLXML_PARSE_ERROR

// On MSVC, restore warnings state
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#endif
