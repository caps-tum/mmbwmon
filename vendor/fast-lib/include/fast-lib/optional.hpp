/*
 * This file is part of fast-lib.
 * Copyright (C) 2015 RWTH Aachen University - ACS
 *
 * This file is licensed under the GNU Lesser General Public License Version 3
 * Version 3, 29 June 2007. For details see 'LICENSE.md' in the root directory.
 */

#ifndef FAST_LIB_OPTIONAL_HPP
#define FAST_LIB_OPTIONAL_HPP

#include <fast-lib/serializable.hpp>
#include <yaml-cpp/yaml.h>

#include <memory>

namespace fast
{

// boost.optional?
template<typename T>
class Optional :
	public fast::Serializable
{
public:
	using value_type = T;

	Optional(std::string tag) noexcept;
	Optional(std::string tag, std::unique_ptr<T> ptr) noexcept;

	Optional(std::string tag, const T &val);
	Optional(const Optional<T> &rhs);
	Optional(Optional<T> &&rhs) noexcept;
	Optional<T> & operator=(const T &rhs);
	Optional<T> & operator=(T &&rhs);
	Optional<T> & operator=(Optional<T> rhs);

	bool operator==(const Optional<T> &rhs) const;
	bool operator==(const T &rhs) const;

	operator T&();
	operator const T&() const;

	bool is_valid() const noexcept;

	T & get();
	const T & get() const;

	std::string get_tag() const;

	void set(const T &rhs);
	void set(T &&rhs);

	YAML::Node emit() const override;
	void load(const YAML::Node &node) override;
private:
	std::string tag;
	std::unique_ptr<T> ptr;
	bool valid;
};

template<typename T>
Optional<T>::Optional(std::string tag) noexcept :
	tag(std::move(tag)),
	valid(false)
{
}

template<typename T>
Optional<T>::Optional(std::string tag, std::unique_ptr<T> ptr) noexcept :
	tag(std::move(tag)),
	ptr(std::move(ptr)),
	valid(true)
{
}

template<typename T>
Optional<T>::Optional(std::string tag, const T &rhs) :
	tag(std::move(tag)),
	ptr(new T(rhs)),
	valid(true)
{
}

template<typename T>
Optional<T>::Optional(const Optional<T> &rhs) :
	tag(rhs.tag),
	ptr(rhs.is_valid() ? new T(*rhs.ptr) : nullptr),
	valid(rhs.valid)
{
}

template<typename T>
Optional<T>::Optional(Optional<T> &&rhs) noexcept :
	tag(std::move(rhs.tag)),
	ptr(std::move(rhs.ptr)),
	valid(rhs.valid)
{
	rhs.valid = false;
}

template<typename T>
Optional<T> & Optional<T>::operator=(const T &rhs)
{
	ptr.reset(new T(rhs));
	valid = true;
	return *this;
}

template<typename T>
Optional<T> & Optional<T>::operator=(T &&rhs)
{
	ptr.reset(new T(std::move(rhs)));
	valid = true;
	return *this;
}

template<typename T>
Optional<T> & Optional<T>::operator=(Optional<T> rhs)
{
	tag = std::move(rhs.tag);
	valid = rhs.valid;
	ptr = std::move(rhs.ptr);
	return *this;
}

template<typename T>
bool Optional<T>::operator==(const Optional<T> &rhs) const
{
	if (is_valid() != rhs.is_valid())
		return false;
	if (is_valid())
		return get() == rhs.get();
	return true;
}

template<typename T>
bool Optional<T>::operator==(const T &rhs) const
{
	return is_valid() && (get() == rhs);
}

template<typename T>
Optional<T>::operator T & ()
{
	return get();
}

template<typename T>
Optional<T>::operator const T & () const
{
	return get();
}

template<typename T>
bool Optional<T>::is_valid() const noexcept
{
	return valid;
}

template<typename T>
T & Optional<T>::get()
{
	if (!valid)
		throw std::runtime_error("Optional value not valid.");
	return *ptr;
}

template<typename T>
const T & Optional<T>::get() const
{
	if (!valid)
		throw std::runtime_error("Optional value not valid.");
	return *ptr;
}

template<typename T>
std::string Optional<T>::get_tag() const
{
	return tag;
}

template<typename T>
void Optional<T>::set(const T &rhs)
{
	ptr.reset(new T(rhs));
	valid = true;
}

template<typename T>
void Optional<T>::set(T &&rhs)
{
	ptr.reset(new T(std::move(rhs)));
	valid = true;
}

template<typename T>
YAML::Node Optional<T>::emit() const
{
	YAML::Node node;
	if (valid)
		node[tag] = *ptr;
	return node;
}

template<typename T>
void Optional<T>::load(const YAML::Node &node)
{
	if (tag != "" && node[tag]) {
		ptr.reset(new T(node[tag].template as<T>()));
		valid = true;
	}
}

}
#endif
