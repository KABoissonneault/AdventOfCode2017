#pragma once

#include <system_error>
#include <expected.hpp>
#include <string>
#include <string_view>

namespace kab_advent {
	class error_info {
	public:
		error_info() = default;
		error_info(std::error_code err)
			: m_err(err) {

		}
		error_info(std::error_code err, std::string_view message)
			: m_err(err)
			, m_str(message) {

		}

		auto get_error_code() const noexcept -> std::error_code { return m_err; }
		auto get_error_message() const noexcept -> std::string_view { return m_str; }

	private:
		std::error_code m_err;
		std::string m_str;
	};

	auto operator<<(std::ostream& o, error_info const& error) -> std::ostream& {
		if (error.get_error_message().size() > 0) {
			o << error.get_error_code().message() << ": " << error.get_error_message();
		}
		else {
			o << error.get_error_code().message();
		}
		return o;
	}

	template<typename T, typename E=error_info>
	using expected = tl::expected<T, E>;
	template<typename E>
	using unexpected = tl::unexpected<E>;
	using tl::unexpect;
}