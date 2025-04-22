//
// Created by dave on 29/07/2024.
//

#ifndef RAPIDXML_RAPIDXML_PREDICATES_HPP
#define RAPIDXML_RAPIDXML_PREDICATES_HPP

#include <string_view>
#include <list>
#include <flxml/generator.h>
#include <flxml.h>

namespace flxml {
    template<typename Ch> class xpath;
    namespace internal {
        template<typename Ch>
        class xpath_base;

        template<typename Ch=char>
        class name : public flxml::internal::xpath_base<Ch> {
        private:
            std::basic_string<Ch> m_name;
            std::optional<std::basic_string<Ch>> m_xmlns;
        public:
            explicit name(std::basic_string_view<Ch> n)
                    : xpath_base<Ch>(), m_name(n) {}

            explicit name(std::basic_string<Ch> const & xmlns, std::basic_string_view<Ch> n)
                    : xpath_base<Ch>(), m_name(n), m_xmlns(xmlns) {}

            bool do_match(const xml_node<Ch> & t) override {
                if (m_xmlns.has_value() && t.xmlns() != m_xmlns.value()) return false;
                return (t.type() == node_type::node_element) && (t.name() == m_name || m_name == "*");
            }
        };

        template<typename Ch=char>
        class value : public flxml::internal::xpath_base<Ch> {
        private:
            std::basic_string<Ch> m_value;
        public:
            explicit value(std::basic_string_view<Ch> v)
                    : xpath_base<Ch>(), m_value(v) {}

            bool do_match(const xml_node<Ch> & t) override {
                return (t.type() == node_type::node_element) && (t.value() == m_value);
            }
        };

        template<typename Ch=char>
        class xmlns : public flxml::internal::xpath_base<Ch> {
        private:
            std::basic_string<Ch> m_xmlns;
        public:
            explicit xmlns(std::basic_string_view<Ch> v)
                    : xpath_base<Ch>(), m_xmlns(v) {}

            bool do_match(const xml_node<Ch> & t) override {
                return (t.type() == node_type::node_element) && (t.xmlns() == m_xmlns);
            }
        };

        template<typename Ch=char>
        class attr : public flxml::internal::xpath_base<Ch> {
        private:
            std::basic_string<Ch> m_name;
            std::basic_string<Ch> m_value;
            std::optional<std::basic_string<Ch>> m_xmlns;
        public:
            explicit attr(std::basic_string_view<Ch> n, std::basic_string_view<Ch> v)
                    : xpath_base<Ch>(), m_name(n), m_value(v) {}

            explicit attr(std::basic_string<Ch> const & x, std::basic_string_view<Ch> n, std::basic_string_view<Ch> v)
                    : xpath_base<Ch>(), m_name(n), m_value(v), m_xmlns(x) {}

            bool do_match(const xml_node<Ch> & t) override {
                if (t.type() != node_type::node_element) return false;
                for (auto const & attr : t.attributes()) {
                    if (m_xmlns.has_value()) {
                        if (m_name == "*" || attr.local_name() != m_name) continue;
                        if (attr.xmlns() != m_xmlns.value()) continue;
                    } else {
                        if (m_name == "*" || attr.name() != m_name) continue;
                    }
                    return attr.value() == m_value;
                }
                return false;
            }
        };

        template<typename Ch=char>
        class root : public flxml::internal::xpath_base<Ch> {
        public:
            root() = default;

            generator<xml_node<Ch> &> do_gather(xml_node<Ch> & t) override {
                for (auto & x : t.children()) {
                    co_yield x;
                }
            }

            bool do_match(const xml_node<Ch> & t) override {
                return t.type() == node_type::node_document || t.type() == node_type::node_element;
            }
        };

        template<typename Ch=char>
        class any : public flxml::internal::xpath_base<Ch> {
        public:
            any() = default;

            generator<xml_node<Ch> &> do_gather(xml_node<Ch> & t) override {
                co_yield t; // self
                for (auto & x : t.descendants()) {
                    co_yield x;
                }
            }

            bool do_match(const xml_node<Ch> & t) override {
                return t.type() == node_type::node_document || t.type() == node_type::node_element;
            }
        };

        template<typename Ch=char>
        class xpath_base {
        private:
            std::list<std::unique_ptr<xpath<Ch>>> m_contexts;
        public:

            xpath_base() = default;

            virtual ~xpath_base() = default;

            virtual generator<xml_node<Ch> &> do_gather(xml_node<Ch> & t) {
                co_yield t;
            }

            generator<xml_node<Ch> &> gather(xml_node<Ch> & t) {
                for (auto & x : do_gather(t)) {
                    if (match(x)) co_yield x;
                }
            }

            virtual bool do_match(const xml_node<Ch> & t) = 0;

            bool match(xml_node<Ch> & t) {
                if (!do_match(t)) {
                    return false;
                }
                for(auto & context : m_contexts) {
                    if (!context->first(t)) {
                        return false;
                    }
                }
                return true;
            }

            void context(std::unique_ptr<xpath<Ch>> && xp) {
                m_contexts.emplace_back(std::move(xp));
            }

            auto & contexts() const {
                return m_contexts;
            }
        };

        std::map<std::string,std::string> xmlns_empty = {};
    }

    template<typename Ch=char>
    class xpath : public internal::xpath_base<Ch> {
    private:
        std::vector<std::unique_ptr<internal::xpath_base<Ch>>> m_chain;
        std::map<std::string,std::string> const & m_xmlns;

    public:
        bool do_match(const xml_node<Ch> & t) override {
            return false;
        }

        auto const & chain() const {
            return m_chain;
        }
        std::string const & prefix_lookup(std::basic_string_view<Ch> const & prefix) const {
            std::basic_string<Ch> p{prefix};
            auto it = m_xmlns.find(p);
            if (it != m_xmlns.end()) {
                return (*it).second;
            }
            throw std::runtime_error("XPath contains unknown prefix");
        }

        static void parse_predicate(std::basic_string_view<Ch> const &name, xpath<Ch> &xp, bool inner) {
            using xml_doc = xml_document<Ch>;
            if (name.starts_with('@')) {
                std::basic_string<Ch> text = "<dummy ";
                bool star = false;
                if (name.starts_with("@*")) {
                    text += "star ";
                    text += name.substr(2);
                    star = true;
                } else {
                    text += name.substr(1);
                }
                text += "/>";
                xml_doc doc;
                doc.template parse<parse_fastest>(text);
                auto attr = doc.first_node()->first_attribute();
                auto colon = attr->name().find(':');
                if (colon != xml_attribute<Ch>::view_type::npos) {
                    auto const & uri = xp.prefix_lookup(attr->name().substr(0, colon));
                    xp.m_chain.push_back(std::make_unique<internal::attr<Ch>>(uri, attr->local_name(), attr->value()));
                } else {
                    xp.m_chain.push_back(std::make_unique<internal::attr<Ch>>(star ? "*" : attr->name(), attr->value()));
                }
            } else if (name.starts_with("text()")) {
                // text match
                std::basic_string<Ch> text = "<dummy text";
                text += name.substr(6);
                text += "/>";
                xml_doc doc;
                doc.template parse<parse_fastest>(text);
                auto attr = doc.first_node()->first_attribute();
                xp.m_chain.push_back(std::make_unique<internal::value<Ch>>(attr->value()));
            } else if (name.starts_with("namespace-uri()")) {
                // text match
                std::basic_string<Ch> text = "<dummy xmlns";
                text += name.substr(6);
                text += "/>";
                xml_doc doc;
                doc.template parse<parse_fastest>(text);
                auto attr = doc.first_node()->first_attribute();
                xp.m_chain.push_back(std::make_unique<internal::xmlns<Ch>>(attr->value()));
            } else {
                if (xp.m_chain.empty() && inner) {
                    xp.m_chain.push_back(std::make_unique<internal::root<Ch>>());
                }
                auto colon = name.find(':');
                if (colon != std::basic_string_view<Ch>::npos) {
                    auto const & uri = xp.prefix_lookup(name.substr(0, colon));
                    xp.m_chain.push_back(std::make_unique<internal::name<Ch>>(uri, name.substr(colon + 1)));
                } else {
                    xp.m_chain.push_back(std::make_unique<internal::name<Ch>>(name));
                }
            }
        }

        static bool parse_inner(std::map<std::string,std::string> & xmlns, std::basic_string_view<Ch> &view, xpath<Ch> &xp, bool first=false, bool inner=false) {
            if (view.starts_with("//")) {
                xp.m_chain.push_back(std::make_unique<internal::any<Ch>>());
                view.remove_prefix(2);
            } else if (view.starts_with('/')) {
                xp.m_chain.push_back(std::make_unique<internal::root<Ch>>());
                view.remove_prefix(1);
            } else if (first && !inner) {
                xp.m_chain.push_back(std::make_unique<internal::any<Ch>>());
            }
            for (typename std::basic_string_view<Ch>::size_type i = 0; i != view.size(); ++i) {
                switch (view[i]) {
                    case '/':
                    case ']':
                        if (i == 0) throw std::runtime_error("Empty name?");
                    case '[':
                        if (i != 0) parse_predicate(view.substr(0, i), xp, inner);
                }
                switch (view[i]) {
                    case ']':
                        view.remove_prefix(i + 1);
                        if (!inner) throw std::runtime_error("Unexpected ] in input");
                        return true;
                    case '[':
                        view.remove_prefix(i + 1);
                        xp.m_chain[xp.m_chain.size() - 1]->context(parse_cont(xmlns, view));
                        return false;
                    case '/':
                        view.remove_prefix(i );
                        return false;
                }
            }
            if (!view.empty()) {
                parse_predicate(view, xp, inner);
                view.remove_prefix(view.length());
            }
            return true;
        }

        static std::unique_ptr<xpath<Ch>> parse_cont(std::map<std::string,std::string> & xmlns, std::basic_string_view<Ch> &view) {
            if (view.empty()) throw std::runtime_error("Context expression is empty");
            auto xp = std::make_unique<xpath<Ch>>(xmlns);
            if (!parse_inner(xmlns, view, *xp, true, true)) {
                while (!view.empty()) {
                    if (parse_inner(xmlns, view, *xp, false, true)) break;
                }
            }
            return xp;
        }

        static std::unique_ptr<xpath<Ch>> parse(std::map<std::string,std::string> & xmlns, std::basic_string_view<Ch> &view) {
            if (view.empty()) throw std::runtime_error("XPath expression is empty");
            auto xp = std::make_unique<xpath<Ch>>(xmlns);
            if (!parse_inner(xmlns, view, *xp, true, false)) {
                while (!view.empty()) {
                    if (parse_inner(xmlns, view, *xp, false, false)) break;
                }
            }
            return xp;
        }
        static std::unique_ptr<xpath<Ch>> parse(std::map<std::string,std::string> & xmlns, std::basic_string_view<Ch> const &view) {
            std::basic_string_view<Ch> sv(view);
            return parse(xmlns, sv);
        }
        static std::unique_ptr<xpath<Ch>> parse(std::map<std::string,std::string> & xmlns, std::basic_string<Ch> const &view) {
            std::basic_string_view<Ch> sv(view);
            return parse(xmlns, sv);
        }
        static std::unique_ptr<xpath<Ch>> parse(std::map<std::string,std::string> & xmlns, const char * view) {
            std::basic_string_view<Ch> sv(view);
            return parse(xmlns, sv);
        }
        static std::unique_ptr<xpath<Ch>> parse(std::basic_string_view<Ch> &sv) {
            return parse(internal::xmlns_empty, sv);
        }
        static std::unique_ptr<xpath<Ch>> parse(std::basic_string<Ch> const &view) {
            std::basic_string_view<Ch> sv(view);
            return parse(internal::xmlns_empty, sv);
        }
        static std::unique_ptr<xpath<Ch>> parse(std::basic_string_view<Ch> const &view) {
            std::basic_string_view<Ch> sv(view);
            return parse(internal::xmlns_empty, sv);
        }
        static std::unique_ptr<xpath<Ch>> parse(const char * view) {
            std::basic_string_view<Ch> sv(view);
            return parse(internal::xmlns_empty, sv);
        }

        explicit xpath(std::map<std::string,std::string> & xmlns) : m_xmlns(xmlns) {}

        flxml::generator<xml_node<Ch> &> all(xml_node<Ch> & current, unsigned int depth = 0) {
            if (depth >= m_chain.size()) throw std::logic_error("Depth exceeded");
            auto & xp = m_chain[depth];
            depth++;
            for (auto & r : xp->gather(current)) {
                if (depth >= m_chain.size()) {
                    co_yield r;
                } else {
                    for (auto & t : all(r, depth)) {
                        co_yield t;
                    }
                }
            }
        }

        xml_node<Ch>::ptr first(xml_node<Ch> & current) {
            for (auto &r: all(current)) {
                return &r;
            }
            return {};
        }
    };
}

#endif //RAPIDXML_RAPIDXML_PREDICATES_HPP
