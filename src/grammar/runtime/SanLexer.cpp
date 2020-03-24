
// Generated from SanLexer.g4 by ANTLR 4.8


#include "SanLexer.h"


using namespace antlr4;


SanLexer::SanLexer(CharStream *input) : Lexer(input) {
  _interpreter = new atn::LexerATNSimulator(this, _atn, _decisionToDFA, _sharedContextCache);
}

SanLexer::~SanLexer() {
  delete _interpreter;
}

std::string SanLexer::getGrammarFileName() const {
  return "SanLexer.g4";
}

const std::vector<std::string>& SanLexer::getRuleNames() const {
  return _ruleNames;
}

const std::vector<std::string>& SanLexer::getChannelNames() const {
  return _channelNames;
}

const std::vector<std::string>& SanLexer::getModeNames() const {
  return _modeNames;
}

const std::vector<std::string>& SanLexer::getTokenNames() const {
  return _tokenNames;
}

dfa::Vocabulary& SanLexer::getVocabulary() const {
  return _vocabulary;
}

const std::vector<uint16_t> SanLexer::getSerializedATN() const {
  return _serializedATN;
}

const atn::ATN& SanLexer::getATN() const {
  return _atn;
}




// Static vars and initialization.
std::vector<dfa::DFA> SanLexer::_decisionToDFA;
atn::PredictionContextCache SanLexer::_sharedContextCache;

// We own the ATN which in turn owns the ATN states.
atn::ATN SanLexer::_atn;
std::vector<uint16_t> SanLexer::_serializedATN;

std::vector<std::string> SanLexer::_ruleNames = {
  u8"Add", u8"Sub", u8"Mul", u8"Div", u8"Mod", u8"OpeningParen", u8"ClosingParen", 
  u8"OpeningBrace", u8"ClosingBrace", u8"OpeningBracket", u8"ClosingBracket", 
  u8"StringLiteral", u8"CharLiteral", u8"StringChar", u8"IntegerLiteral", 
  u8"DecimalLiteral", u8"ZeroLiteral", u8"HexadecimalLiteral", u8"BinaryLiteral", 
  u8"WhiteSpace", u8"LineTerminator", u8"Escape", u8"DIGITSEPARATOR", u8"NONDIGIT", 
  u8"DIGIT", u8"NONZERODIGIT", u8"HEXADECIMALDIGIT", u8"BINARYDIGIT"
};

std::vector<std::string> SanLexer::_channelNames = {
  "DEFAULT_TOKEN_CHANNEL", "HIDDEN"
};

std::vector<std::string> SanLexer::_modeNames = {
  u8"DEFAULT_MODE"
};

std::vector<std::string> SanLexer::_literalNames = {
  "", u8"'+'", u8"'-'", u8"'*'", u8"'/'", u8"'%'", u8"'('", u8"')'", u8"'{'", 
  u8"'}'", u8"'['", u8"']'", "", "", "", "", u8"'0'"
};

std::vector<std::string> SanLexer::_symbolicNames = {
  "", u8"Add", u8"Sub", u8"Mul", u8"Div", u8"Mod", u8"OpeningParen", u8"ClosingParen", 
  u8"OpeningBrace", u8"ClosingBrace", u8"OpeningBracket", u8"ClosingBracket", 
  u8"StringLiteral", u8"CharLiteral", u8"IntegerLiteral", u8"DecimalLiteral", 
  u8"ZeroLiteral", u8"HexadecimalLiteral", u8"BinaryLiteral", u8"WhiteSpace", 
  u8"LineTerminator"
};

dfa::Vocabulary SanLexer::_vocabulary(_literalNames, _symbolicNames);

std::vector<std::string> SanLexer::_tokenNames;

SanLexer::Initializer::Initializer() {
  // This code could be in a static initializer lambda, but VS doesn't allow access to private class members from there.
	for (size_t i = 0; i < _symbolicNames.size(); ++i) {
		std::string name = _vocabulary.getLiteralName(i);
		if (name.empty()) {
			name = _vocabulary.getSymbolicName(i);
		}

		if (name.empty()) {
			_tokenNames.push_back("<INVALID>");
		} else {
      _tokenNames.push_back(name);
    }
	}

  _serializedATN = {
    0x3, 0x608b, 0xa72a, 0x8133, 0xb9ed, 0x417c, 0x3be7, 0x7786, 0x5964, 
    0x2, 0x16, 0xbb, 0x8, 0x1, 0x4, 0x2, 0x9, 0x2, 0x4, 0x3, 0x9, 0x3, 0x4, 
    0x4, 0x9, 0x4, 0x4, 0x5, 0x9, 0x5, 0x4, 0x6, 0x9, 0x6, 0x4, 0x7, 0x9, 
    0x7, 0x4, 0x8, 0x9, 0x8, 0x4, 0x9, 0x9, 0x9, 0x4, 0xa, 0x9, 0xa, 0x4, 
    0xb, 0x9, 0xb, 0x4, 0xc, 0x9, 0xc, 0x4, 0xd, 0x9, 0xd, 0x4, 0xe, 0x9, 
    0xe, 0x4, 0xf, 0x9, 0xf, 0x4, 0x10, 0x9, 0x10, 0x4, 0x11, 0x9, 0x11, 
    0x4, 0x12, 0x9, 0x12, 0x4, 0x13, 0x9, 0x13, 0x4, 0x14, 0x9, 0x14, 0x4, 
    0x15, 0x9, 0x15, 0x4, 0x16, 0x9, 0x16, 0x4, 0x17, 0x9, 0x17, 0x4, 0x18, 
    0x9, 0x18, 0x4, 0x19, 0x9, 0x19, 0x4, 0x1a, 0x9, 0x1a, 0x4, 0x1b, 0x9, 
    0x1b, 0x4, 0x1c, 0x9, 0x1c, 0x4, 0x1d, 0x9, 0x1d, 0x3, 0x2, 0x3, 0x2, 
    0x3, 0x3, 0x3, 0x3, 0x3, 0x4, 0x3, 0x4, 0x3, 0x5, 0x3, 0x5, 0x3, 0x6, 
    0x3, 0x6, 0x3, 0x7, 0x3, 0x7, 0x3, 0x8, 0x3, 0x8, 0x3, 0x9, 0x3, 0x9, 
    0x3, 0xa, 0x3, 0xa, 0x3, 0xb, 0x3, 0xb, 0x3, 0xc, 0x3, 0xc, 0x3, 0xd, 
    0x3, 0xd, 0x7, 0xd, 0x54, 0xa, 0xd, 0xc, 0xd, 0xe, 0xd, 0x57, 0xb, 0xd, 
    0x3, 0xd, 0x3, 0xd, 0x3, 0xe, 0x3, 0xe, 0x6, 0xe, 0x5d, 0xa, 0xe, 0xd, 
    0xe, 0xe, 0xe, 0x5e, 0x3, 0xe, 0x3, 0xe, 0x3, 0xf, 0x3, 0xf, 0x5, 0xf, 
    0x65, 0xa, 0xf, 0x3, 0x10, 0x3, 0x10, 0x3, 0x10, 0x3, 0x10, 0x5, 0x10, 
    0x6b, 0xa, 0x10, 0x3, 0x11, 0x3, 0x11, 0x5, 0x11, 0x6f, 0xa, 0x11, 0x3, 
    0x11, 0x7, 0x11, 0x72, 0xa, 0x11, 0xc, 0x11, 0xe, 0x11, 0x75, 0xb, 0x11, 
    0x3, 0x12, 0x3, 0x12, 0x3, 0x13, 0x3, 0x13, 0x3, 0x13, 0x3, 0x13, 0x5, 
    0x13, 0x7d, 0xa, 0x13, 0x3, 0x13, 0x3, 0x13, 0x5, 0x13, 0x81, 0xa, 0x13, 
    0x3, 0x13, 0x7, 0x13, 0x84, 0xa, 0x13, 0xc, 0x13, 0xe, 0x13, 0x87, 0xb, 
    0x13, 0x3, 0x14, 0x3, 0x14, 0x3, 0x14, 0x3, 0x14, 0x5, 0x14, 0x8d, 0xa, 
    0x14, 0x3, 0x14, 0x3, 0x14, 0x5, 0x14, 0x91, 0xa, 0x14, 0x3, 0x14, 0x7, 
    0x14, 0x94, 0xa, 0x14, 0xc, 0x14, 0xe, 0x14, 0x97, 0xb, 0x14, 0x3, 0x15, 
    0x6, 0x15, 0x9a, 0xa, 0x15, 0xd, 0x15, 0xe, 0x15, 0x9b, 0x3, 0x15, 0x3, 
    0x15, 0x3, 0x16, 0x3, 0x16, 0x3, 0x16, 0x3, 0x16, 0x3, 0x17, 0x3, 0x17, 
    0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 0x3, 
    0x17, 0x3, 0x17, 0x5, 0x17, 0xae, 0xa, 0x17, 0x3, 0x18, 0x3, 0x18, 0x3, 
    0x19, 0x3, 0x19, 0x3, 0x1a, 0x3, 0x1a, 0x3, 0x1b, 0x3, 0x1b, 0x3, 0x1c, 
    0x3, 0x1c, 0x3, 0x1d, 0x3, 0x1d, 0x2, 0x2, 0x1e, 0x3, 0x3, 0x5, 0x4, 
    0x7, 0x5, 0x9, 0x6, 0xb, 0x7, 0xd, 0x8, 0xf, 0x9, 0x11, 0xa, 0x13, 0xb, 
    0x15, 0xc, 0x17, 0xd, 0x19, 0xe, 0x1b, 0xf, 0x1d, 0x2, 0x1f, 0x10, 0x21, 
    0x11, 0x23, 0x12, 0x25, 0x13, 0x27, 0x14, 0x29, 0x15, 0x2b, 0x16, 0x2d, 
    0x2, 0x2f, 0x2, 0x31, 0x2, 0x33, 0x2, 0x35, 0x2, 0x37, 0x2, 0x39, 0x2, 
    0x3, 0x2, 0xb, 0x6, 0x2, 0xc, 0xc, 0xf, 0xf, 0x24, 0x24, 0x5e, 0x5e, 
    0x4, 0x2, 0xb, 0xb, 0x22, 0x22, 0x4, 0x2, 0xc, 0xc, 0xf, 0xf, 0x4, 0x2, 
    0x29, 0x29, 0x61, 0x61, 0x5, 0x2, 0x43, 0x5c, 0x61, 0x61, 0x63, 0x7c, 
    0x3, 0x2, 0x32, 0x3b, 0x3, 0x2, 0x33, 0x3b, 0x5, 0x2, 0x32, 0x3b, 0x43, 
    0x48, 0x63, 0x68, 0x3, 0x2, 0x32, 0x33, 0x2, 0xc5, 0x2, 0x3, 0x3, 0x2, 
    0x2, 0x2, 0x2, 0x5, 0x3, 0x2, 0x2, 0x2, 0x2, 0x7, 0x3, 0x2, 0x2, 0x2, 
    0x2, 0x9, 0x3, 0x2, 0x2, 0x2, 0x2, 0xb, 0x3, 0x2, 0x2, 0x2, 0x2, 0xd, 
    0x3, 0x2, 0x2, 0x2, 0x2, 0xf, 0x3, 0x2, 0x2, 0x2, 0x2, 0x11, 0x3, 0x2, 
    0x2, 0x2, 0x2, 0x13, 0x3, 0x2, 0x2, 0x2, 0x2, 0x15, 0x3, 0x2, 0x2, 0x2, 
    0x2, 0x17, 0x3, 0x2, 0x2, 0x2, 0x2, 0x19, 0x3, 0x2, 0x2, 0x2, 0x2, 0x1b, 
    0x3, 0x2, 0x2, 0x2, 0x2, 0x1f, 0x3, 0x2, 0x2, 0x2, 0x2, 0x21, 0x3, 0x2, 
    0x2, 0x2, 0x2, 0x23, 0x3, 0x2, 0x2, 0x2, 0x2, 0x25, 0x3, 0x2, 0x2, 0x2, 
    0x2, 0x27, 0x3, 0x2, 0x2, 0x2, 0x2, 0x29, 0x3, 0x2, 0x2, 0x2, 0x2, 0x2b, 
    0x3, 0x2, 0x2, 0x2, 0x3, 0x3b, 0x3, 0x2, 0x2, 0x2, 0x5, 0x3d, 0x3, 0x2, 
    0x2, 0x2, 0x7, 0x3f, 0x3, 0x2, 0x2, 0x2, 0x9, 0x41, 0x3, 0x2, 0x2, 0x2, 
    0xb, 0x43, 0x3, 0x2, 0x2, 0x2, 0xd, 0x45, 0x3, 0x2, 0x2, 0x2, 0xf, 0x47, 
    0x3, 0x2, 0x2, 0x2, 0x11, 0x49, 0x3, 0x2, 0x2, 0x2, 0x13, 0x4b, 0x3, 
    0x2, 0x2, 0x2, 0x15, 0x4d, 0x3, 0x2, 0x2, 0x2, 0x17, 0x4f, 0x3, 0x2, 
    0x2, 0x2, 0x19, 0x51, 0x3, 0x2, 0x2, 0x2, 0x1b, 0x5a, 0x3, 0x2, 0x2, 
    0x2, 0x1d, 0x64, 0x3, 0x2, 0x2, 0x2, 0x1f, 0x6a, 0x3, 0x2, 0x2, 0x2, 
    0x21, 0x6c, 0x3, 0x2, 0x2, 0x2, 0x23, 0x76, 0x3, 0x2, 0x2, 0x2, 0x25, 
    0x7c, 0x3, 0x2, 0x2, 0x2, 0x27, 0x8c, 0x3, 0x2, 0x2, 0x2, 0x29, 0x99, 
    0x3, 0x2, 0x2, 0x2, 0x2b, 0x9f, 0x3, 0x2, 0x2, 0x2, 0x2d, 0xad, 0x3, 
    0x2, 0x2, 0x2, 0x2f, 0xaf, 0x3, 0x2, 0x2, 0x2, 0x31, 0xb1, 0x3, 0x2, 
    0x2, 0x2, 0x33, 0xb3, 0x3, 0x2, 0x2, 0x2, 0x35, 0xb5, 0x3, 0x2, 0x2, 
    0x2, 0x37, 0xb7, 0x3, 0x2, 0x2, 0x2, 0x39, 0xb9, 0x3, 0x2, 0x2, 0x2, 
    0x3b, 0x3c, 0x7, 0x2d, 0x2, 0x2, 0x3c, 0x4, 0x3, 0x2, 0x2, 0x2, 0x3d, 
    0x3e, 0x7, 0x2f, 0x2, 0x2, 0x3e, 0x6, 0x3, 0x2, 0x2, 0x2, 0x3f, 0x40, 
    0x7, 0x2c, 0x2, 0x2, 0x40, 0x8, 0x3, 0x2, 0x2, 0x2, 0x41, 0x42, 0x7, 
    0x31, 0x2, 0x2, 0x42, 0xa, 0x3, 0x2, 0x2, 0x2, 0x43, 0x44, 0x7, 0x27, 
    0x2, 0x2, 0x44, 0xc, 0x3, 0x2, 0x2, 0x2, 0x45, 0x46, 0x7, 0x2a, 0x2, 
    0x2, 0x46, 0xe, 0x3, 0x2, 0x2, 0x2, 0x47, 0x48, 0x7, 0x2b, 0x2, 0x2, 
    0x48, 0x10, 0x3, 0x2, 0x2, 0x2, 0x49, 0x4a, 0x7, 0x7d, 0x2, 0x2, 0x4a, 
    0x12, 0x3, 0x2, 0x2, 0x2, 0x4b, 0x4c, 0x7, 0x7f, 0x2, 0x2, 0x4c, 0x14, 
    0x3, 0x2, 0x2, 0x2, 0x4d, 0x4e, 0x7, 0x5d, 0x2, 0x2, 0x4e, 0x16, 0x3, 
    0x2, 0x2, 0x2, 0x4f, 0x50, 0x7, 0x5f, 0x2, 0x2, 0x50, 0x18, 0x3, 0x2, 
    0x2, 0x2, 0x51, 0x55, 0x7, 0x24, 0x2, 0x2, 0x52, 0x54, 0x5, 0x1d, 0xf, 
    0x2, 0x53, 0x52, 0x3, 0x2, 0x2, 0x2, 0x54, 0x57, 0x3, 0x2, 0x2, 0x2, 
    0x55, 0x53, 0x3, 0x2, 0x2, 0x2, 0x55, 0x56, 0x3, 0x2, 0x2, 0x2, 0x56, 
    0x58, 0x3, 0x2, 0x2, 0x2, 0x57, 0x55, 0x3, 0x2, 0x2, 0x2, 0x58, 0x59, 
    0x7, 0x24, 0x2, 0x2, 0x59, 0x1a, 0x3, 0x2, 0x2, 0x2, 0x5a, 0x5c, 0x7, 
    0x29, 0x2, 0x2, 0x5b, 0x5d, 0x5, 0x1d, 0xf, 0x2, 0x5c, 0x5b, 0x3, 0x2, 
    0x2, 0x2, 0x5d, 0x5e, 0x3, 0x2, 0x2, 0x2, 0x5e, 0x5c, 0x3, 0x2, 0x2, 
    0x2, 0x5e, 0x5f, 0x3, 0x2, 0x2, 0x2, 0x5f, 0x60, 0x3, 0x2, 0x2, 0x2, 
    0x60, 0x61, 0x7, 0x29, 0x2, 0x2, 0x61, 0x1c, 0x3, 0x2, 0x2, 0x2, 0x62, 
    0x65, 0xa, 0x2, 0x2, 0x2, 0x63, 0x65, 0x5, 0x2d, 0x17, 0x2, 0x64, 0x62, 
    0x3, 0x2, 0x2, 0x2, 0x64, 0x63, 0x3, 0x2, 0x2, 0x2, 0x65, 0x1e, 0x3, 
    0x2, 0x2, 0x2, 0x66, 0x6b, 0x5, 0x21, 0x11, 0x2, 0x67, 0x6b, 0x5, 0x23, 
    0x12, 0x2, 0x68, 0x6b, 0x5, 0x25, 0x13, 0x2, 0x69, 0x6b, 0x5, 0x27, 
    0x14, 0x2, 0x6a, 0x66, 0x3, 0x2, 0x2, 0x2, 0x6a, 0x67, 0x3, 0x2, 0x2, 
    0x2, 0x6a, 0x68, 0x3, 0x2, 0x2, 0x2, 0x6a, 0x69, 0x3, 0x2, 0x2, 0x2, 
    0x6b, 0x20, 0x3, 0x2, 0x2, 0x2, 0x6c, 0x73, 0x5, 0x35, 0x1b, 0x2, 0x6d, 
    0x6f, 0x5, 0x2f, 0x18, 0x2, 0x6e, 0x6d, 0x3, 0x2, 0x2, 0x2, 0x6e, 0x6f, 
    0x3, 0x2, 0x2, 0x2, 0x6f, 0x70, 0x3, 0x2, 0x2, 0x2, 0x70, 0x72, 0x5, 
    0x33, 0x1a, 0x2, 0x71, 0x6e, 0x3, 0x2, 0x2, 0x2, 0x72, 0x75, 0x3, 0x2, 
    0x2, 0x2, 0x73, 0x71, 0x3, 0x2, 0x2, 0x2, 0x73, 0x74, 0x3, 0x2, 0x2, 
    0x2, 0x74, 0x22, 0x3, 0x2, 0x2, 0x2, 0x75, 0x73, 0x3, 0x2, 0x2, 0x2, 
    0x76, 0x77, 0x7, 0x32, 0x2, 0x2, 0x77, 0x24, 0x3, 0x2, 0x2, 0x2, 0x78, 
    0x79, 0x7, 0x32, 0x2, 0x2, 0x79, 0x7d, 0x7, 0x7a, 0x2, 0x2, 0x7a, 0x7b, 
    0x7, 0x32, 0x2, 0x2, 0x7b, 0x7d, 0x7, 0x5a, 0x2, 0x2, 0x7c, 0x78, 0x3, 
    0x2, 0x2, 0x2, 0x7c, 0x7a, 0x3, 0x2, 0x2, 0x2, 0x7d, 0x7e, 0x3, 0x2, 
    0x2, 0x2, 0x7e, 0x85, 0x5, 0x37, 0x1c, 0x2, 0x7f, 0x81, 0x5, 0x2f, 0x18, 
    0x2, 0x80, 0x7f, 0x3, 0x2, 0x2, 0x2, 0x80, 0x81, 0x3, 0x2, 0x2, 0x2, 
    0x81, 0x82, 0x3, 0x2, 0x2, 0x2, 0x82, 0x84, 0x5, 0x37, 0x1c, 0x2, 0x83, 
    0x80, 0x3, 0x2, 0x2, 0x2, 0x84, 0x87, 0x3, 0x2, 0x2, 0x2, 0x85, 0x83, 
    0x3, 0x2, 0x2, 0x2, 0x85, 0x86, 0x3, 0x2, 0x2, 0x2, 0x86, 0x26, 0x3, 
    0x2, 0x2, 0x2, 0x87, 0x85, 0x3, 0x2, 0x2, 0x2, 0x88, 0x89, 0x7, 0x32, 
    0x2, 0x2, 0x89, 0x8d, 0x7, 0x64, 0x2, 0x2, 0x8a, 0x8b, 0x7, 0x32, 0x2, 
    0x2, 0x8b, 0x8d, 0x7, 0x44, 0x2, 0x2, 0x8c, 0x88, 0x3, 0x2, 0x2, 0x2, 
    0x8c, 0x8a, 0x3, 0x2, 0x2, 0x2, 0x8d, 0x8e, 0x3, 0x2, 0x2, 0x2, 0x8e, 
    0x95, 0x5, 0x39, 0x1d, 0x2, 0x8f, 0x91, 0x5, 0x2f, 0x18, 0x2, 0x90, 
    0x8f, 0x3, 0x2, 0x2, 0x2, 0x90, 0x91, 0x3, 0x2, 0x2, 0x2, 0x91, 0x92, 
    0x3, 0x2, 0x2, 0x2, 0x92, 0x94, 0x5, 0x39, 0x1d, 0x2, 0x93, 0x90, 0x3, 
    0x2, 0x2, 0x2, 0x94, 0x97, 0x3, 0x2, 0x2, 0x2, 0x95, 0x93, 0x3, 0x2, 
    0x2, 0x2, 0x95, 0x96, 0x3, 0x2, 0x2, 0x2, 0x96, 0x28, 0x3, 0x2, 0x2, 
    0x2, 0x97, 0x95, 0x3, 0x2, 0x2, 0x2, 0x98, 0x9a, 0x9, 0x3, 0x2, 0x2, 
    0x99, 0x98, 0x3, 0x2, 0x2, 0x2, 0x9a, 0x9b, 0x3, 0x2, 0x2, 0x2, 0x9b, 
    0x99, 0x3, 0x2, 0x2, 0x2, 0x9b, 0x9c, 0x3, 0x2, 0x2, 0x2, 0x9c, 0x9d, 
    0x3, 0x2, 0x2, 0x2, 0x9d, 0x9e, 0x8, 0x15, 0x2, 0x2, 0x9e, 0x2a, 0x3, 
    0x2, 0x2, 0x2, 0x9f, 0xa0, 0x9, 0x4, 0x2, 0x2, 0xa0, 0xa1, 0x3, 0x2, 
    0x2, 0x2, 0xa1, 0xa2, 0x8, 0x16, 0x3, 0x2, 0xa2, 0x2c, 0x3, 0x2, 0x2, 
    0x2, 0xa3, 0xa4, 0x7, 0x5e, 0x2, 0x2, 0xa4, 0xae, 0x7, 0x29, 0x2, 0x2, 
    0xa5, 0xa6, 0x7, 0x5e, 0x2, 0x2, 0xa6, 0xae, 0x7, 0x24, 0x2, 0x2, 0xa7, 
    0xa8, 0x7, 0x5e, 0x2, 0x2, 0xa8, 0xae, 0x7, 0x5e, 0x2, 0x2, 0xa9, 0xaa, 
    0x7, 0x5e, 0x2, 0x2, 0xaa, 0xae, 0x7, 0x70, 0x2, 0x2, 0xab, 0xac, 0x7, 
    0x5e, 0x2, 0x2, 0xac, 0xae, 0x7, 0x74, 0x2, 0x2, 0xad, 0xa3, 0x3, 0x2, 
    0x2, 0x2, 0xad, 0xa5, 0x3, 0x2, 0x2, 0x2, 0xad, 0xa7, 0x3, 0x2, 0x2, 
    0x2, 0xad, 0xa9, 0x3, 0x2, 0x2, 0x2, 0xad, 0xab, 0x3, 0x2, 0x2, 0x2, 
    0xae, 0x2e, 0x3, 0x2, 0x2, 0x2, 0xaf, 0xb0, 0x9, 0x5, 0x2, 0x2, 0xb0, 
    0x30, 0x3, 0x2, 0x2, 0x2, 0xb1, 0xb2, 0x9, 0x6, 0x2, 0x2, 0xb2, 0x32, 
    0x3, 0x2, 0x2, 0x2, 0xb3, 0xb4, 0x9, 0x7, 0x2, 0x2, 0xb4, 0x34, 0x3, 
    0x2, 0x2, 0x2, 0xb5, 0xb6, 0x9, 0x8, 0x2, 0x2, 0xb6, 0x36, 0x3, 0x2, 
    0x2, 0x2, 0xb7, 0xb8, 0x9, 0x9, 0x2, 0x2, 0xb8, 0x38, 0x3, 0x2, 0x2, 
    0x2, 0xb9, 0xba, 0x9, 0xa, 0x2, 0x2, 0xba, 0x3a, 0x3, 0x2, 0x2, 0x2, 
    0x11, 0x2, 0x55, 0x5e, 0x64, 0x6a, 0x6e, 0x73, 0x7c, 0x80, 0x85, 0x8c, 
    0x90, 0x95, 0x9b, 0xad, 0x4, 0x8, 0x2, 0x2, 0x2, 0x3, 0x2, 
  };

  atn::ATNDeserializer deserializer;
  _atn = deserializer.deserialize(_serializedATN);

  size_t count = _atn.getNumberOfDecisions();
  _decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    _decisionToDFA.emplace_back(_atn.getDecisionState(i), i);
  }
}

SanLexer::Initializer SanLexer::_init;
