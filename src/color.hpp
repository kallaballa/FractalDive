#ifndef SRC_COLOR_HPP_
#define SRC_COLOR_HPP_

#include <cstdint>

namespace fractaldive {

template<typename T> struct Color {
	constexpr Color() : r_(0), g_(0), b_(0) {
	}

	constexpr Color(const T& r, const T& g, const T& b) : r_(r), g_(g), b_(b) {
	}
	constexpr Color(const uint32_t& rgb) : r_(rgb >> 16 & 0xFF), g_(rgb >> 8 & 0xFF), b_(rgb & 0xFF) {
	}

	T r_;
	T g_;
	T b_;
};

	constexpr static Color<uint8_t> PALETTE[256] = {
			Color<uint8_t>(206),
			Color<uint8_t>(10027161),
			Color<uint8_t>(8855943),
			Color<uint8_t>(8388736),
			Color<uint8_t>(7607180),
			Color<uint8_t>(5870940),
			Color<uint8_t>(2326940),
			Color<uint8_t>(10066431),
			Color<uint8_t>(9857534),
			Color<uint8_t>(9799916),
			Color<uint8_t>(9540095),
			Color<uint8_t>(8812777),
			Color<uint8_t>(8553215),
			Color<uint8_t>(7961087),
			Color<uint8_t>(7566335),
			Color<uint8_t>(6772195),
			Color<uint8_t>(6329563),
			Color<uint8_t>(5724159),
			Color<uint8_t>(4818390),
			Color<uint8_t>(3744726),
			Color<uint8_t>(2713272),
			Color<uint8_t>(2660279),
			Color<uint8_t>(8990436),
			Color<uint8_t>(10091895),
			Color<uint8_t>(10084607),
			Color<uint8_t>(10078207),
			Color<uint8_t>(9830223),
			Color<uint8_t>(9694890),
			Color<uint8_t>(9682838),
			Color<uint8_t>(9633529),
			Color<uint8_t>(9043043),
			Color<uint8_t>(8830207),
			Color<uint8_t>(8435076),
			Color<uint8_t>(7994446),
			Color<uint8_t>(7728381),
			Color<uint8_t>(7722751),
			Color<uint8_t>(7714047),
			Color<uint8_t>(7667448),
			Color<uint8_t>(7649964),
			Color<uint8_t>(7536277),
			Color<uint8_t>(6801378),
			Color<uint8_t>(6749952),
			Color<uint8_t>(6547964),
			Color<uint8_t>(6487502),
			Color<uint8_t>(6476031),
			Color<uint8_t>(6466047),
			Color<uint8_t>(6418944),
			Color<uint8_t>(5889792),
			Color<uint8_t>(5749977),
			Color<uint8_t>(5439264),
			Color<uint8_t>(4782861),
			Color<uint8_t>(4502741),
			Color<uint8_t>(3600896),
			Color<uint8_t>(3407296),
			Color<uint8_t>(3333888),
			Color<uint8_t>(2731990),
			Color<uint8_t>(2612821),
			Color<uint8_t>(2465989),
			Color<uint8_t>(2416891),
			Color<uint8_t>(10329344),
			Color<uint8_t>(10178281),
			Color<uint8_t>(10093566),
			Color<uint8_t>(9437039),
			Color<uint8_t>(9436893),
			Color<uint8_t>(9361130),
			Color<uint8_t>(9352422),
			Color<uint8_t>(9291707),
			Color<uint8_t>(9246891),
			Color<uint8_t>(9236477),
			Color<uint8_t>(9228774),
			Color<uint8_t>(9211135),
			Color<uint8_t>(9174696),
			Color<uint8_t>(9100543),
			Color<uint8_t>(8256983),
			Color<uint8_t>(8186776),
			Color<uint8_t>(8112865),
			Color<uint8_t>(8103905),
			Color<uint8_t>(7339844),
			Color<uint8_t>(7121008),
			Color<uint8_t>(6974207),
			Color<uint8_t>(449787),
			Color<uint8_t>(6291191),
			Color<uint8_t>(6205086),
			Color<uint8_t>(5987327),
			Color<uint8_t>(5225949),
			Color<uint8_t>(4980344),
			Color<uint8_t>(4907889),
			Color<uint8_t>(4887942),
			Color<uint8_t>(4056316),
			Color<uint8_t>(258987),
			Color<uint8_t>(256934),
			Color<uint8_t>(3123918),
			Color<uint8_t>(3110096),
			Color<uint8_t>(3000320),
			Color<uint8_t>(2096883),
			Color<uint8_t>(2083658),
			Color<uint8_t>(2066599),
			Color<uint8_t>(130287),
			Color<uint8_t>(127806),
			Color<uint8_t>(16777215),
			Color<uint8_t>(16777213),
			Color<uint8_t>(16777187),
			Color<uint8_t>(16777175),
			Color<uint8_t>(16777160),
			Color<uint8_t>(16777141),
			Color<uint8_t>(16777130),
			Color<uint8_t>(16777113),
			Color<uint8_t>(16777092),
			Color<uint8_t>(16776957),
			Color<uint8_t>(16776955),
			Color<uint8_t>(16776951),
			Color<uint8_t>(16776703),
			Color<uint8_t>(16776701),
			Color<uint8_t>(16776697),
			Color<uint8_t>(16776695),
			Color<uint8_t>(16776159),
			Color<uint8_t>(16775914),
			Color<uint8_t>(16775679),
			Color<uint8_t>(16775678),
			Color<uint8_t>(16775676),
			Color<uint8_t>(16775668),
			Color<uint8_t>(16775658),
			Color<uint8_t>(16775630),
			Color<uint8_t>(16775142),
			Color<uint8_t>(16775095),
			Color<uint8_t>(16774615),
			Color<uint8_t>(16774378),
			Color<uint8_t>(16773874),
			Color<uint8_t>(16773849),
			Color<uint8_t>(16773842),
			Color<uint8_t>(16773764),
			Color<uint8_t>(16773606),
			Color<uint8_t>(16773574),
			Color<uint8_t>(16773226),
			Color<uint8_t>(16772861),
			Color<uint8_t>(16772859),
			Color<uint8_t>(16772351),
			Color<uint8_t>(16772341),
			Color<uint8_t>(16772332),
			Color<uint8_t>(16772272),
			Color<uint8_t>(16772057),
			Color<uint8_t>(16771818),
			Color<uint8_t>(16771780),
			Color<uint8_t>(16771767),
			Color<uint8_t>(16771360),
			Color<uint8_t>(16770768),
			Color<uint8_t>(16770741),
			Color<uint8_t>(16770713),
			Color<uint8_t>(16770047),
			Color<uint8_t>(16769736),
			Color<uint8_t>(16769478),
			Color<uint8_t>(16769177),
			Color<uint8_t>(16769023),
			Color<uint8_t>(16769016),
			Color<uint8_t>(16769007),
			Color<uint8_t>(16768991),
			Color<uint8_t>(16768674),
			Color<uint8_t>(16768373),
			Color<uint8_t>(16767995),
			Color<uint8_t>(16767415),
			Color<uint8_t>(16766899),
			Color<uint8_t>(16766342),
			Color<uint8_t>(16765775),
			Color<uint8_t>(16765026),
			Color<uint8_t>(16764836),
			Color<uint8_t>(16764671),
			Color<uint8_t>(16764622),
			Color<uint8_t>(16764578),
			Color<uint8_t>(16764531),
			Color<uint8_t>(16763695),
			Color<uint8_t>(16763641),
			Color<uint8_t>(16763122),
			Color<uint8_t>(16763107),
			Color<uint8_t>(16763080),
			Color<uint8_t>(16763029),
			Color<uint8_t>(16762952),
			Color<uint8_t>(16762459),
			Color<uint8_t>(16762111),
			Color<uint8_t>(16761998),
			Color<uint8_t>(16760648),
			Color<uint8_t>(16760360),
			Color<uint8_t>(16760194),
			Color<uint8_t>(16759807),
			Color<uint8_t>(16759799),
			Color<uint8_t>(16759773),
			Color<uint8_t>(16759739),
			Color<uint8_t>(16759677),
			Color<uint8_t>(16758283),
			Color<uint8_t>(16758197),
			Color<uint8_t>(16757800),
			Color<uint8_t>(16755948),
			Color<uint8_t>(16755810),
			Color<uint8_t>(16755552),
			Color<uint8_t>(16754943),
			Color<uint8_t>(16754899),
			Color<uint8_t>(16754856),
			Color<uint8_t>(16753919),
			Color<uint8_t>(16753828),
			Color<uint8_t>(16752714),
			Color<uint8_t>(16751682),
			Color<uint8_t>(16750568),
			Color<uint8_t>(16750539),
			Color<uint8_t>(16750487),
			Color<uint8_t>(16749361),
			Color<uint8_t>(16748174),
			Color<uint8_t>(16747146),
			Color<uint8_t>(16746239),
			Color<uint8_t>(16746211),
			Color<uint8_t>(16746178),
			Color<uint8_t>(16744461),
			Color<uint8_t>(16743935),
			Color<uint8_t>(16742881),
			Color<uint8_t>(16741749),
			Color<uint8_t>(16741305),
			Color<uint8_t>(16741235),
			Color<uint8_t>(16738525),
			Color<uint8_t>(16736944),
			Color<uint8_t>(16733011),
			Color<uint8_t>(16730879),
			Color<uint8_t>(16730184),
			Color<uint8_t>(16723455),
			Color<uint8_t>(16721446),
			Color<uint8_t>(16711418),
			Color<uint8_t>(16711167),
			Color<uint8_t>(16710395),
			Color<uint8_t>(16689651),
			Color<uint8_t>(16685297),
			Color<uint8_t>(16681968),
			Color<uint8_t>(16672747),
			Color<uint8_t>(16646143),
			Color<uint8_t>(16646141),
			Color<uint8_t>(16645887),
			Color<uint8_t>(16645631),
			Color<uint8_t>(16645625),
			Color<uint8_t>(16645616),
			Color<uint8_t>(16645369),
			Color<uint8_t>(16645115),
			Color<uint8_t>(16644857),
			Color<uint8_t>(16644607),
			Color<uint8_t>(16644605),
			Color<uint8_t>(16642815),
			Color<uint8_t>(16579817),
			Color<uint8_t>(16579061),
			Color<uint8_t>(16578549),
			Color<uint8_t>(16514555),
			Color<uint8_t>(16514024),
			Color<uint8_t>(16513535),
			Color<uint8_t>(16449527),
			Color<uint8_t>(16449275),
			Color<uint8_t>(16449022),
			Color<uint8_t>(16448479),
			Color<uint8_t>(16446191),
			Color<uint8_t>(16444671),
			Color<uint8_t>(16443372),
			Color<uint8_t>(16383995),
			Color<uint8_t>(16383743)
		};
} /* namespace fractaldive */

#endif /* SRC_COLOR_HPP_ */
