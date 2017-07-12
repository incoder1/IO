#ifndef __IO_NET_ERROR_CATEGORY_SHIM_HPP_INCLUDED__
#define __IO_NET_ERROR_CATEGORY_SHIM_HPP_INCLUDED__

#include "../config.hpp"
#include "../conststring.hpp"

#ifndef IO_HAS_BOOST
#	error "io network channels requares boost assio please build with boost"
#endif // IO_HAS_BOOST

#include <boost/system/error_code.hpp>

#include <unordered_map>
#include <system_error>
#include <mutex>

namespace io {

typedef boost::system::error_category boost_err_cat;

class boost_error_category_shim final : public std::error_category {
public:
	boost_error_category_shim(const boost_err_cat* berrcat) noexcept;
	virtual ~boost_error_category_shim() noexcept = default;
	virtual const char* name() const noexcept override;
	virtual std::error_condition default_error_condition(int err) const noexcept override;
	virtual bool equivalent (const std::error_code& code, int condition) const noexcept override;
	virtual inline std::string message(int err_code) const override
	{
		return std::string( name_.data() );
	}
private:
	const boost_err_cat *berrcat_;
	const_string name_;
};

struct berrcat_equal_to : public
		std::binary_function<
			const boost_err_cat*,
			const boost_err_cat*,
			bool>
{
	typedef std::binary_function<
			const boost_err_cat*,
			const boost_err_cat*,
			bool> super_type;

	constexpr berrcat_equal_to() noexcept:
		super_type()
	{}

    constexpr bool operator() (const boost_err_cat* rhs,const boost_err_cat* lsh) const noexcept
    {
     	return rhs == lsh;
	}
};

class boost_error_code_cache {
	boost_error_code_cache(const boost_error_code_cache&) = delete;
	boost_error_code_cache& operator=(const boost_error_code_cache&) = delete;
private:
	typedef std::unique_ptr<boost_error_category_shim> u_category_shim;
	typedef std::pair<const boost_err_cat*,u_category_shim> map_pair_t;

	typedef std::unordered_map<
				const boost_err_cat*,
				u_category_shim,
				std::hash<const boost_err_cat*>,
				berrcat_equal_to,
				h_allocator< map_pair_t, memory_traits >
			> conversion_map;

	typedef typename conversion_map::const_iterator map_citerator;

	boost_error_code_cache() noexcept;
public:
	static boost_error_code_cache* instance() noexcept;
	const std::error_category* find(const boost_err_cat *berrcat) noexcept;
private:
	friend class nobadalloc<boost_error_code_cache>;
	static boost_error_code_cache* volatile _instance;
	static std::mutex _mtx;

	conversion_map cache_;
};

void convert_error_codes(std::error_code& ec, const boost::system::error_code& bec) noexcept;

} // namespace io


#endif // __IO_NET_ERROR_CATEGORY_SHIM_HPP_INCLUDED__
