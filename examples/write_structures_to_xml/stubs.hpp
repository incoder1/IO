#ifndef STUBS_HPP_INCLUDED
#define STUBS_HPP_INCLUDED

#include <xml_types.hpp>

#ifdef HAS_PRAGMA_ONCE
#pragma once
#endif // HAS_PRAGMA_ONCE

class config {
public:

	typedef std::chrono::time_point<std::chrono::system_clock>  date_t;

	config() noexcept:
		id_(0),
		enabled_(false),
		time_created_(),
		name_()
	{}

	config(uint8_t id, bool enabled, date_t&& tc, std::string&& name) noexcept:
		id_(id),
		enabled_(enabled),
		time_created_( std::forward< date_t > (tc) ),
		name_(std::forward<std::string>(name))
	{}

	inline uint8_t id() const noexcept {
		return id_;
	}

	inline bool enabled() const noexcept {
		return enabled_;
	}

	inline const date_t& time_created() const noexcept {
		return time_created_;
	}

	inline const date_t& date_created() const noexcept {
		return time_created_;
	}

	inline const std::string& name() const noexcept {
		return name_;
	}

	typedef io::xml::complex_type<config,
			std::tuple<io::xml::byte_attribute,io::xml::bool_attribute>,
			std::tuple<io::xml::date_element,io::xml::string_element>
	    > xml_type;

	xml_type to_xml_type() const
	{
		// attributes
		io::xml::byte_attribute id("id", id_);
		io::xml::bool_attribute en("enabled", enabled_);
		// elements
		io::xml::date_element tm("date-created", time_created_);
		io::xml::string_element s("name", name_ );
		return xml_type("configuration", std::make_tuple(id,en), std::make_tuple(tm,s) );
	}

	static config from_xml_type(const xml_type& xt)
	{
		// attributes
		uint8_t id = std::get<0>( xt.attributes() ).value();
		bool enabled = std::get<1>( xt.attributes() ).value();
		// elements
		std::chrono::time_point<std::chrono::system_clock> tm = std::get<0>( xt.elements() ).value();
		std::string msg( std::get<1>( xt.elements() ).value() );
		return config(id, enabled, std::move(tm) , std::move(msg) );
	}

private:
	uint8_t id_;
	bool enabled_;
	std::chrono::time_point<std::chrono::system_clock> time_created_;
	std::string name_;
};

struct primary_conf
{
public:
	explicit primary_conf(uint8_t id) noexcept:
		id_(id)
	{}

	inline uint8_t id() const noexcept {
		return id_;
	}

	typedef io::xml::complex_type<
				 primary_conf,
				 std::tuple<io::xml::byte_attribute>,
				 std::tuple<>
			  >
			xml_type;

	xml_type to_xml_type() const {
		return xml_type( "primary-configuration",
							std::make_tuple( io::xml::byte_attribute("id",id_) ),
							std::tuple<>()
						);
	}

	static primary_conf from_xml_type(const xml_type* xt) {
		return primary_conf( std::get<0>(xt->attributes()).value() );
	}

private:
	uint8_t id_;
};

class app_settings {
private:
	// element's list
	typedef io::xml::list_type< config::xml_type > config_list;

	app_settings(primary_conf&& primary, std::vector<config>&& confs) noexcept:
		primary_conf_(std::forward<primary_conf>(primary)),
		confs_(std::forward< std::vector<config> >(confs))
	{}

public:

	static constexpr const char* XSI = "http://www.w3.org/2001/XMLSchema-instance";
	static constexpr const char* SCHEMA = "app-config.xsd";

	app_settings(primary_conf&& primary):
		primary_conf_(std::move(primary)),
		confs_()
	{}

	primary_conf primary() const noexcept {
		return primary_conf_;
	}

	void add_conf(config&& conf) {
		confs_.emplace_back( std::forward<config>(conf) );
	}

	std::vector<config>::const_iterator first_config() const noexcept {
		return confs_.cbegin();
	}

	std::vector<config>::const_iterator end_config() const noexcept {
		return confs_.cend();
	}

	typedef io::xml::complex_type<
				app_settings,
				std::tuple<
					   io::xml::string_attribute,
					   io::xml::string_attribute>,
				std::tuple<
						primary_conf::xml_type,
						config_list>
			> xml_type;

	xml_type to_xml_type() const
	{
		io::xml::string_attribute xsi("xmlns:xsi", XSI );
		io::xml::string_attribute sh("xsi:noNamespaceSchemaLocation", SCHEMA);
		config_list confs("configurations");
		confs.add_elements("configuration", confs_.cbegin(), confs_.cend() );
		return xml_type("application-settings",
						std::make_tuple(xsi,sh),
						std::make_tuple( primary_conf_.to_xml_type(), std::move(confs) ) );
	}

	static app_settings from_xml_type(const xml_type& xt)
	{
		// elements
		primary_conf::xml_type r =  std::get<0>( xt.elements() );
		primary_conf rt = primary_conf::from_xml_type( &r );
		// get and unmap container
		config_list confs( std::get<1>(xt.elements()) );
		std::vector<config> vconf;
		confs.unmap( vconf );
		return app_settings( std::move(rt), std::move(vconf) );
	}
private:
	primary_conf primary_conf_;
	std::vector<config> confs_;
};

#endif // STUBS_HPP_INCLUDED
