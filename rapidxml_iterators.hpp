#ifndef RAPIDXML_ITERATORS_HPP_INCLUDED
#define RAPIDXML_ITERATORS_HPP_INCLUDED

// Copyright (C) 2006, 2009 Marcin Kalicinski
// Version 1.13
// Revision $DateTime: 2009/05/13 01:46:17 $
//! \file rapidxml_iterators.hpp This file contains rapidxml iterators

#include "rapidxml.hpp"

namespace rapidxml
{
    //! Iterator of child nodes of xml_node
    template<typename Ch>
class node_iterator
    {
    public:
        using value_type = xml_node<Ch>;
        using reference = xml_node<Ch> &;
        using pointer = xml_node<Ch> *;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = long;

        node_iterator()
            : m_node()
        {
        }

        explicit node_iterator(xml_node<Ch> const &node)
            : m_node(node.first_node())
        {
        }

        node_iterator(node_iterator && other)  noexcept : m_node(other.m_node) {}
        node_iterator(node_iterator const & other) : m_node(other.m_node) {}

        reference operator *() const
        {
            return const_cast<reference>(*m_node);
        }

        pointer operator->() const
        {
            return const_cast<pointer>(m_node.get());
        }

        node_iterator& operator++()
        {
            m_node = m_node->next_sibling();
            return *this;
        }

        node_iterator operator++(int)
        {
            node_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        node_iterator& operator--()
        {
            m_node = m_node->previous_sibling();
            return *this;
        }

        node_iterator operator--(int)
        {
            node_iterator tmp = *this;
            --(*this);
            return tmp;
        }

        bool operator == (const node_iterator<Ch>& rhs) const
        {
            return m_node == rhs.m_node;
        }

        bool operator != (const node_iterator<Ch>& rhs) const
        {
            return m_node != rhs.m_node;
        }

        node_iterator & operator = (node_iterator && other)  noexcept {
            m_node = other.m_node;
            return *this;
        }

        node_iterator & operator = (node_iterator const & other) {
            m_node = other.m_node;
            return *this;
        }

        bool valid()
        {
            return m_node.has_value();
        }

    private:

        optional_ptr<xml_node<Ch>> m_node;

    };

    //! Iterator of child attributes of xml_node
    template<class Ch>
    class attribute_iterator
    {
    
    public:

        using value_type = xml_attribute<Ch>;
        using reference = xml_attribute<Ch> &;
        using pointer = xml_attribute<Ch> *;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = long;

        attribute_iterator()
            : m_attribute()
        {
        }

        explicit attribute_iterator(xml_node<Ch> const &node)
            : m_attribute(node.first_attribute())
        {
        }

        attribute_iterator(attribute_iterator && other)  noexcept : m_attribute(other.m_attribute) {}
        attribute_iterator(attribute_iterator const & other) : m_attribute(other.m_attribute) {}

        reference operator *() const
        {
            return const_cast<reference>(*m_attribute);
        }

        pointer operator->() const
        {
            return const_cast<pointer>(m_attribute.get());
        }

        attribute_iterator& operator++()
        {
            m_attribute = m_attribute->next_attribute();
            return *this;
        }

        attribute_iterator operator++(int)
        {
            attribute_iterator tmp = *this;
            ++*this;
            return tmp;
        }

        attribute_iterator& operator--()
        {
            m_attribute = m_attribute->previous_attribute();
            return *this;
        }

        attribute_iterator operator--(int)
        {
            attribute_iterator tmp = *this;
            --*this;
            return tmp;
        }

        bool operator ==(const attribute_iterator<Ch> &rhs) const
        {
            return m_attribute == rhs.m_attribute;
        }

        bool operator !=(const attribute_iterator<Ch> &rhs) const
        {
            return m_attribute != rhs.m_attribute;
        }

        attribute_iterator & operator = (attribute_iterator && other)  noexcept {
            m_attribute = other.m_attribute;
            return *this;
        }

        attribute_iterator & operator = (attribute_iterator const & other) {
            m_attribute = other.m_attribute;
            return *this;
        }

    private:

        optional_ptr<xml_attribute<Ch>> m_attribute;

    };

    //! Container adaptor for child nodes
    template<typename Ch>
    class children
    {
        xml_node<Ch> const & m_node;
    public:
        explicit children(xml_node<Ch> const & node) : m_node(node) {}
        explicit children(optional_ptr<xml_node<Ch>> const ptr) : m_node(ptr.value()) {}
        children(children && other)  noexcept : m_node(other.m_node) {}
        children(children const & other) : m_node(other.m_node) {}

        using const_iterator = node_iterator<Ch>;
        using iterator = node_iterator<Ch>;

        iterator begin() {
            return iterator(m_node);
        }
        iterator end() {
            return {};
        }
        const_iterator begin() const {
            return const_iterator(m_node);
        }
        const_iterator end() const {
            return {};
        }
    };

    //! Container adaptor for attributes
    template<typename Ch>
    class attributes
    {
        xml_node<Ch> const & m_node;
    public:
        explicit attributes(xml_node<Ch> const & node) : m_node(node) {}
        explicit attributes(optional_ptr<xml_node<Ch>> ptr) : m_node(ptr.value()) {}

        using const_iterator = attribute_iterator<Ch>;
        using iterator = attribute_iterator<Ch>;

        iterator begin() {
            return iterator{m_node};
        }
        iterator end() {
            return {};
        }
        const_iterator begin() const {
            return const_iterator{m_node};
        }
        const_iterator end() const {
            return {};
        }
    };
}

template<typename Ch>
inline constexpr bool std::ranges::enable_borrowed_range<rapidxml::children<Ch>> = true;

template<typename Ch>
inline constexpr bool std::ranges::enable_borrowed_range<rapidxml::attributes<Ch>> = true;


#endif
