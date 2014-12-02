#ifndef _MY_FUNCTION_HPP_a211fa14_f6df_46e6_b712_c2e78dfba141
#define _MY_FUNCTION_HPP_a211fa14_f6df_46e6_b712_c2e78dfba141
#include <map>
using namespace std;

template<typename _Kty, typename _Vty>
_Vty& findAndInsertIfNotExists(map<_Kty, _Vty>& data, const _Kty& key) {
	typename map<_Kty, _Vty>::iterator itor = data.find(key);
	if(data.end() == itor) {
		itor = data.insert(pair<_Kty, _Vty>(key, _Vty())).first;
	}
	return itor->second;
}

#endif
