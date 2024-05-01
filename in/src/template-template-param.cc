// template-template-param.cc
// Simple example of a template template parameter.

// Based on example at:
// https://en.cppreference.com/w/cpp/language/template_parameters

template <typename T>
class my_array {};

// two type template parameters and one template template parameter:
template <typename K,
          typename V,
          template <typename> typename C = my_array>
class Map
{
  C<K> key;
  C<V> value;
};

Map<int, float> m;

// EOF
