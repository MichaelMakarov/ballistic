#pragma once

namespace math {
	template<typename> 
	struct function_type {
		constexpr static bool is_function_ptr = false;
	};

	template<typename Res, typename ... Args>
	struct function_type<Res(*)(Args...)> {
		constexpr static bool is_function_ptr = true;
	};

	template<typename> struct function_signature;

	template<typename Res, typename ... Args>
	struct function_signature<Res(*)(Args...)> {
		using signature_t = Res(*)(Args...);
	};

	template<typename> struct method_signature;

	template<typename Obj, typename Res, typename ... Args>
	struct method_signature<Res(Obj::*)(Args...)> : public function_signature<Res(*)(Args...)> {};

	template<typename Obj, typename Res, typename ... Args>
	struct method_signature<Res(Obj::*)(Args...) const> : public function_signature<Res(*)(Args...)> {};

	template<bool, typename> struct function_ptr;

	template<typename Func>
	struct function_ptr<true, Func> : public function_signature<Func> {};

	template<typename Func>
	struct function_ptr<false, Func> : public method_signature<decltype(&Func::operator())> {};

	template<typename Func>
	struct callable_deducer : function_ptr<function_type<Func>::is_function_ptr, Func> {};
}