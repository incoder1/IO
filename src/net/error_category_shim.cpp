#include "stdafx.hpp"
#include "error_category_shim.hpp"

namespace io {

static const boost_err_cat* address_of(const boost_err_cat& errcat)
{
	return reinterpret_cast<const boost_err_cat*>
		       ( & const_cast<uint8_t&>(
					reinterpret_cast<const volatile uint8_t&>(errcat)
				  )
			);
}

// boost_error_code_cache

boost_error_code_cache* volatile boost_error_code_cache::_instance = nullptr;
std::mutex boost_error_code_cache::_mtx;

boost_error_code_cache* boost_error_code_cache::instance() noexcept
{
	if(nullptr == _instance) {
		_mtx.lock();
		 if(nullptr == _instance) {
			std::error_code ec;
			_instance = nobadalloc<boost_error_code_cache>::construct(ec);
			if(ec) {
				_mtx.unlock();
				std::terminate();
			}
		 }
		_mtx.unlock();
	}
	return _instance;
}

boost_error_code_cache::boost_error_code_cache() noexcept:
	cache_()
{}

const std::error_category* boost_error_code_cache::find(const boost_err_cat *berrcat) noexcept
{
		map_citerator i = cache_.find( berrcat );
		_mtx.lock();
#ifndef IO_NO_EXCEPTIONS
		try {
#endif // IO_NO_EXCEPTIONS
			std::error_code ec;
			u_category_shim cat( nobadalloc<boost_error_category_shim>::construct(ec,berrcat) );
			if(ec)
				std::terminate();
			i = cache_.emplace( berrcat, std::move(cat)  ).first;
#ifndef IO_NO_EXCEPTIONS
		} catch (std::exception& exc) {
#else
		if( i == cache_.cend() ) {
#endif // IO_NO_EXCEPTIONS
			_mtx.unlock();
			std::terminate();
		}
		_mtx.unlock();
		return i->second.get();
}

static const std::error_category* convert_error_category(const boost_err_cat* berrcat)
{
	return boost_error_code_cache::instance()->find( berrcat );
}

// boost_error_category_shim
boost_error_category_shim::boost_error_category_shim(const boost_err_cat* berrcat) noexcept:
	berrcat_( berrcat ),
	name_( berrcat->name() )
{}

const char* boost_error_category_shim::name() const noexcept
{
	return name_.data();
}

std::error_condition boost_error_category_shim::default_error_condition(int errv) const noexcept
{
	const boost::system::error_condition berrcon = berrcat_->default_error_condition(errv);
	const boost_err_cat& deferrcat = berrcon.category();
	const boost_err_cat *cat = address_of( deferrcat );
	// We have to convert the error category here since it may not have the same category as errv.
	return std::error_condition( berrcon.value(), *convert_error_category( cat ) );
}

bool  boost_error_category_shim::equivalent(const std::error_code& code, int condition) const noexcept
{
	return static_cast<int>(this->default_error_condition(code.value()).value()) == condition;
}


void convert_error_codes(std::error_code& ec, const boost::system::error_code& bec) noexcept
{
	if( bec.category() == boost::system::system_category() ) {
		ec.assign(bec.value(), std::system_category() );
	} else if(bec.category() == boost::system::generic_category()) {
		ec.assign(bec.value(), std::generic_category() );
	} else {
		ec.assign( bec.value(), *convert_error_category( address_of(bec.category()) ) );
	}
}

} // namespace io
