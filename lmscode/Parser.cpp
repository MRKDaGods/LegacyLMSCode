#include "Parser.h"
#include "Compiler.h"

#define FORMAT_STR(frmt, ...) const char *emodel = frmt; char *buf = new char[strlen(emodel) * 2]; sscanf(buf, emodel, __VA_ARGS__)

#define CODE_OFFSET(offset) (*code)[offset]
#define SUBMIT_TOKEN token.Length = offset - token.StartIndex; tokens.push_back(token); _STD cout << "Submitted token, " << enumToString(token.TokenContextualType) << _STD endl; state = PARSER_STATE_BEGIN;
#define PARSER_STATE_BEGIN 0
#define PARSER_STATE_IDENTIFIER 1
#define PARSER_STATE_NUM 2
#define PARSER_STATE_DIRECTIVE 3
#define PARSER_STATE_SYMBOL 4

#define PARSER_TOKEN_STATE_NONE 0
#define PARSER_TOKEN_STATE_DIRECTIVE_USE 1
#define PARSER_TOKEN_STATE_DIRECTIVE_NMS 2
#define PARSER_TOKEN_STATE_IDENTIFIER 3
#define PARSER_TOKEN_STATE_CLASS 4
#define PARSER_TOKEN_STATE_FIELD 5
#define PARSER_TOKEN_STATE_METHOD 6
#define PARSER_TOKEN_STATE_METHOD_BODY 7
#define PARSER_TOKEN_STATE_VAR 8

namespace LMS
{
	static const int DirectivesCount = 4;
	static DirectiveInfo *Directives = new DirectiveInfo[DirectivesCount]
	{
		DirectiveInfo { DirectiveType::Use, "use" },
		DirectiveInfo { DirectiveType::Def, "def" },
		DirectiveInfo{ DirectiveType::Nms, "nms" }, //#nms <namespace>
		DirectiveInfo{ DirectiveType::Lms, "LMS" }
	};
	static const int KeywordsCount = 9;
	static KeywordInfo *Keywords = new KeywordInfo[KeywordsCount]
	{
		KeywordInfo { KeywordType::Class, "class" },
		KeywordInfo { KeywordType::Elif, "elif" },
		KeywordInfo { KeywordType::Else, "else" },
		KeywordInfo { KeywordType::If, "if" },
		KeywordInfo { KeywordType::Object, "obj" },
		KeywordInfo { KeywordType::Routine, "routine" },
		KeywordInfo { KeywordType::Field, "fld" },
		KeywordInfo { KeywordType::Method, "func" },
		KeywordInfo { KeywordType::Var, "var" }
	};
	static const int ModifiersCount = 10;
	static ModifierInfo *Modifiers = new ModifierInfo[ModifiersCount]
	{
		ModifierInfo { ModifierType::Private, "private" },
		ModifierInfo { ModifierType::Public, "public" },
		ModifierInfo { ModifierType::Protected, "protected" },
		ModifierInfo { ModifierType::Internal, "internal" },
		ModifierInfo { ModifierType::Abstract, "abstract" },
		ModifierInfo { ModifierType::Virtual, "virtual" },
		ModifierInfo { ModifierType::Override, "override" },
		ModifierInfo { ModifierType::Static, "static" },
		ModifierInfo { ModifierType::Native, "native" },
		ModifierInfo { ModifierType::Entrypoint, "__entrypoint" }
	};

	const char *enumToString(ParserTokenContextualType type)
	{
		switch (type)
		{
		case ParserTokenContextualType::Sbyte:
			return "Sbyte";
		case ParserTokenContextualType::Byte:
			return "Byte";
		case ParserTokenContextualType::Short:
			return "Short";
		case ParserTokenContextualType::UShort:
			return "UShort";
		case ParserTokenContextualType::Int:
			return "Int";
		case ParserTokenContextualType::UInt:
			return "UInt";
		case ParserTokenContextualType::Long:
			return "Long";
		case ParserTokenContextualType::ULong:
			return "ULong";
		case ParserTokenContextualType::Double:
			return "Double";
		case ParserTokenContextualType::Float:
			return "Float";
		case ParserTokenContextualType::Bool:
			return "Bool";
		case ParserTokenContextualType::String:
			return "String";
		case ParserTokenContextualType::LeftParen:
			return "LeftParen";
		case ParserTokenContextualType::RightParen:
			return "RightParen";
		case ParserTokenContextualType::OpenBrace:
			return "OpenBrace";
		case ParserTokenContextualType::ClosedBrace:
			return "ClosedBrace";
		case ParserTokenContextualType::Comma:
			return "Comma";
		case ParserTokenContextualType::Asterisk:
			return "Asterisk";
		case ParserTokenContextualType::Dot:
			return "Dot";
		case ParserTokenContextualType::Keyword:
			return "Keyword";
		}
		return "Unknown";
	}

	Parser::Parser(_STD vector<ParserSourceFileInfo> srcFiles, const char *libname)
	{
		m_Libname = libname;
		for (int i = 0; i < srcFiles.size(); i++)
			m_SrcFiles.push_back(ParserSourceFile{ srcFiles[i] });
	}

	_STD vector<ParserToken> Parser::CollectTokens(ParserSourceFileInfo *src)
	{
		size_t offset = 0;
		_STD string *code = &src->Text;
		int state = PARSER_STATE_BEGIN;
		_STD vector<ParserToken> tokens;
		ParserToken token;
		_STD string buffer;
		int value = -1;
		while (offset < code->size())
		{
			char ch = CODE_OFFSET(offset);
			if (IsIgnorableCharacter(ch))
			{
				if (state != PARSER_STATE_BEGIN)
				{
					switch (state)
					{
					case PARSER_STATE_NUM:
						token.TokenContextualType = ParserTokenContextualType::Int;
						token.IntValue = _STD stoi(buffer);
						break;
					case PARSER_STATE_DIRECTIVE:
						token.BadToken = true;
						token.Info = "Unknown directive";
						break;
					case PARSER_STATE_IDENTIFIER:
						KeywordInfo *kinfo = AsKeyword(buffer);
						ModifierInfo *minfo;
						if (kinfo)
						{
							token.KeywordValue = kinfo;
							token.TokenContextualType = ParserTokenContextualType::Keyword;
						}
						else if (minfo = AsModifier(buffer))
						{
							token.ModifierValue = minfo;
							token.TokenContextualType = ParserTokenContextualType::Modifier;
						}
						else token.TokenContextualType = ParserTokenContextualType::String;
						token.StringValue = buffer;
						break;
					}
					SUBMIT_TOKEN
				}
				state = PARSER_STATE_BEGIN;
				offset++;
				continue;
			}
			switch (state)
			{
			case PARSER_STATE_BEGIN:
				buffer = ""; //clean the buffer
				token = ParserToken(); //reset token
				token.StartIndex = offset;
				if (isdigit(ch))
				{
					token.TokenType = ParserTokenType::Number;
					state = PARSER_STATE_NUM;
					_STD cout << "Parsing num\n";
					offset--;
				}
				else if (isalpha(ch))
				{
					token.TokenType = ParserTokenType::Identifier;
					state = PARSER_STATE_IDENTIFIER;
					offset--;
				}
				else
				{
					token.TokenType = ParserTokenType::Symbol;
					state = PARSER_STATE_SYMBOL;
					offset--;
				}
				break;
			case PARSER_STATE_NUM:
				switch (ch)
				{
				case 'u':
				case 'U':
					switch (CODE_OFFSET(offset + 1))
					{
					case 'l':
					case 'L':
						token.TokenContextualType = ParserTokenContextualType::ULong;
						token.ULongValue = _STD stoul(buffer);
						offset++;
						SUBMIT_TOKEN
							break;
					case 's':
					case 'S':
						token.TokenContextualType = ParserTokenContextualType::UShort;
						value = _STD stoi(buffer);
						if (value > _STD numeric_limits<unsigned short>::max() || value < _STD numeric_limits<unsigned short>::min())
						{
							token.BadToken = true;
							token.Info = buffer + " is outside the boundaries of type 'unsigned short'";
						}
						else token.UShortValue = (unsigned short)value;
						offset++;
						SUBMIT_TOKEN
							break;
					default:
						//uint
						token.TokenContextualType = ParserTokenContextualType::UInt;
						unsigned long val = _STD stoul(buffer);
						if (val > _STD numeric_limits<unsigned int>::max() || val < _STD numeric_limits<unsigned int>::min())
						{
							token.BadToken = true;
							token.Info = buffer + " is outside the boundaries of type 'unsigned int'";
						}
						else token.UIntValue = (unsigned int)val;
						SUBMIT_TOKEN
							break;
					}
					break;
				case 'l':
				case 'L':
					token.TokenContextualType = ParserTokenContextualType::Long;
					token.LongValue = _STD stol(buffer);
					SUBMIT_TOKEN
						break;
				case 's':
				case 'S':
					token.TokenContextualType = ParserTokenContextualType::Short;
					token.ShortValue = _STD stoi(buffer);
					SUBMIT_TOKEN
						break;
				case 'd':
				case 'D':
					token.TokenContextualType = ParserTokenContextualType::Double;
					token.DoubleValue = _STD stod(buffer);
					SUBMIT_TOKEN
						break;
				case 'f':
				case 'F':
					token.TokenContextualType = ParserTokenContextualType::Float;
					token.FloatValue = _STD stof(buffer);
					SUBMIT_TOKEN
						break;
				case 'x':
				case 'X':
					if (buffer.size() == 1 && buffer[0] == '0')
					{
						//hex
						token.IsHex = true;
					}
					else
					{
						token.BadToken = true;
						token.Info = "Unexpected character '" + ch + _STD string("'");
						SUBMIT_TOKEN
					}
					break;
				default:
					if (isdigit(ch) || ch == '.' || (token.IsHex && IsHexCh(ch)))
					{
						_STD cout << "> " << ch << _STD endl;
						buffer += ch;
					}
					else
					{
						token.TokenContextualType = ParserTokenContextualType::Int;
						token.IntValue = _STD stoi(buffer);
						SUBMIT_TOKEN
							offset--;
					}
					break;
				}
				break;
			case PARSER_STATE_SYMBOL:
				switch (ch)
				{
				case '#':
					state = PARSER_STATE_DIRECTIVE;
					break;
				case '(':
					token.TokenContextualType = ParserTokenContextualType::LeftParen;
					SUBMIT_TOKEN
						break;
				case ')':
					token.TokenContextualType = ParserTokenContextualType::RightParen;
					SUBMIT_TOKEN
						break;
				case '{':
					token.TokenContextualType = ParserTokenContextualType::OpenBrace;
					SUBMIT_TOKEN
						break;
				case '}':
					token.TokenContextualType = ParserTokenContextualType::ClosedBrace;
					SUBMIT_TOKEN
						break;
				case ',':
					token.TokenContextualType = ParserTokenContextualType::Comma;
					SUBMIT_TOKEN
						break;
				case '*':
					token.TokenContextualType = ParserTokenContextualType::Asterisk;
					SUBMIT_TOKEN
						break;
				case '.':
					token.TokenContextualType = ParserTokenContextualType::Dot;
					SUBMIT_TOKEN
						break;
				case '=':
					token.TokenContextualType = ParserTokenContextualType::Equal;
					SUBMIT_TOKEN
						break;
				case '+':
					token.TokenContextualType = ParserTokenContextualType::Plus;
					SUBMIT_TOKEN
						break;
				case '-':
					token.TokenContextualType = ParserTokenContextualType::Minus;
					SUBMIT_TOKEN
						break;
				case '/':
					token.TokenContextualType = ParserTokenContextualType::Slash;
					SUBMIT_TOKEN
						break;
				case '|':
					token.TokenContextualType = ParserTokenContextualType::SSlash;
					SUBMIT_TOKEN
						break;
				case ':':
					token.TokenContextualType = ParserTokenContextualType::Colon;
					SUBMIT_TOKEN
						break;
				case '_':
					//until now no logical use for the underscore, so forward to identifiers
					token.TokenType = ParserTokenType::Identifier;
					state = PARSER_STATE_IDENTIFIER;
					offset--;
					break;
				default:
					break;
				}
				break;
			case PARSER_STATE_DIRECTIVE:
				buffer += ch;
				for (int i = 0; i < DirectivesCount; i++)
				{
					DirectiveInfo *info = &Directives[i];
					if (info->Model == buffer)
					{
						token.TokenType = ParserTokenType::Directive;
						token.DirectiveValue = info;
						SUBMIT_TOKEN
							break;
					}
				}
				break;
			case PARSER_STATE_IDENTIFIER:
				if (isalnum(ch) || ch == '_')
					buffer += ch;
				else
				{
					KeywordInfo *kinfo = AsKeyword(buffer);
					ModifierInfo *minfo;
					if (kinfo)
					{
						token.KeywordValue = kinfo;
						token.TokenContextualType = ParserTokenContextualType::Keyword;
					}
					else if (minfo = AsModifier(buffer))
					{
						token.ModifierValue = minfo;
						token.TokenContextualType = ParserTokenContextualType::Modifier;
					}
					else token.TokenContextualType = ParserTokenContextualType::String;
					token.StringValue = buffer;
					SUBMIT_TOKEN
						offset--;
				}
				break;
			}
			offset++;
		}
		return tokens;
	}

	bool Parser::IsIgnorableCharacter(char ch)
	{
		return ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r';
	}

	bool Parser::IsHexCh(char ch)
	{
		return ch == 'A' || ch == 'B' || ch == 'C' || ch == 'D' || ch == 'E' || ch == 'F' || isdigit(ch);
	}

	KeywordInfo *Parser::AsKeyword(_STD string model)
	{
		for (int i = 0; i < KeywordsCount; i++)
		{
			KeywordInfo *info = &Keywords[i];
			if (info->Model == model)
				return info;
		}
		return 0;
	}

	ModifierInfo *Parser::AsModifier(_STD string model)
	{
		if (model == "native")
		{
			int y = 0;
		}
		for (int i = 0; i < ModifiersCount; i++)
		{
			ModifierInfo *info = &Modifiers[i];
			if (info->Model == model)
				return info;
		}
		return 0;
	}

	ParserError Parser::NewError(_STD string msg, size_t sIndex, size_t length, bool isWarning, ParserSourceFileInfo *owner)
	{
		return ParserError
		{
			msg,
			sIndex,
			length,
			isWarning,
			owner
		};
	}

	ExpressionInfo *Parser::ParseExpression(ParserToken *start, int *cIdx, _STD vector<ParserToken> *collection, void **result)
	{
		for (int i = 0; i < ExpressionsCount; i++)
		{
			ExpressionInfo *info = Expressions[i];
			if (info->Expr->Match(start, cIdx, collection, result))
				return info;
		}
		return 0;
	}

	_CLASSINFO *Parser::SearchClass(_STD string name, ParserSourceFile *current, _CLASSINFO *clazzScope)
	{
		_CLASSINFO *vinfo = 0;
		_STD string nextNms = clazzScope->namespaze;
		size_t maxAttempts = current->NamespacesUsed.size() + 1;
		size_t attempts = 0;
		while (!vinfo)
		{
			if (attempts >= maxAttempts)
				break;
			vinfo = _CORE Registry::FindClass(name, nextNms);
			if (vinfo)
				break;
			//if (current->NamespacesUsed.size() > attempts + 1)
			//	nextNms = current->NamespacesUsed[attempts];
			nextNms = current->NamespacesUsed[attempts];
			attempts++;
		}
		return vinfo;
	}

	ParserResult *Parser::Start()
	{
		_CORE Registry::Clean();
		ParserResult *ret = new ParserResult;
		m_CurrentResult = ret;
		ClassToken *globalClass;
		ParserSourceFile *SRC; //debug purpose
		for (int i = 0; i < m_SrcFiles.size(); i++)
		{
			ParserSourceFile *current = &m_SrcFiles[i];
			m_CurrentSourceFile = current;
			SRC = current;
			ParserSourceFileInfo *currentInfo = &current->Info;
			_STD vector<ParserToken> tokens = CollectTokens(currentInfo);
			int state = PARSER_TOKEN_STATE_NONE;
			_STD string currentNms;
			ModifierType currentAccessBuffer = ModifierType::MNone;
			ClassToken *currentClass = 0;
			MethodToken *currentMethod = 0;
			MethodToken mtoken;
			size_t elapsedOBraces = 0;
			size_t elapsedCBraces = 0;
			void *exprres = 0;
			ExpressionInfo *exprinfo = 0;
			int toIgnore = -1;
			for (int j = 0; j < tokens.size(); j++)
			{
				switch (tokens[j].TokenContextualType)
				{
				case ParserTokenContextualType::OpenBrace:
					elapsedOBraces++;
					break;
				case ParserTokenContextualType::ClosedBrace:
					elapsedCBraces++;
					break;
				}
			}
			if (elapsedOBraces != elapsedCBraces)
			{
				ret->Errors.push_back(NewError(elapsedOBraces > elapsedCBraces ? "Expected '}'" : "Expected '{'", 0, 0, false, currentInfo));
			}
			elapsedOBraces = 0;
			elapsedCBraces = 0;
			for (int j = 0; j < tokens.size(); j++)
			{
				if (j == toIgnore)
					continue;
				ParserToken *token = &tokens[j];
				if (j == 0)
				{
					if (token->TokenType != ParserTokenType::Directive)
					{
						ret->Errors.push_back(NewError("Code must begin with the #LMS directive", 0, 0, false, currentInfo));
						break;
					}
					if (!token->DirectiveValue)
					{
						ret->Errors.push_back(NewError("Unexpected error, DIRECTIVE_NULL, Id: 0", 0, 0, false, currentInfo));
						break;
					}
					if (token->DirectiveValue->Type != DirectiveType::Lms)
					{
						ret->Errors.push_back(NewError("Code must begin with the #LMS directive", 0, 0, false, currentInfo));
						break;
					}
					continue;
				}
				switch (state)
				{
				case PARSER_TOKEN_STATE_NONE:
					//begin
					switch (token->TokenType)
					{
					case ParserTokenType::Identifier:
						state = PARSER_TOKEN_STATE_IDENTIFIER;
						j--;
						break;
					case ParserTokenType::Directive:
						switch (token->DirectiveValue->Type)
						{
						case DirectiveType::Use:
							state = PARSER_TOKEN_STATE_DIRECTIVE_USE;
							break;
						case DirectiveType::Nms:
							state = state = PARSER_TOKEN_STATE_DIRECTIVE_NMS;
							break;
						}
						break;
					case ParserTokenType::Symbol:
					{
						if (currentMethod)
						{
							void *res = 0;
							ExpressionInfo *info = ParseExpression(token, &j, &tokens, &res);
							if (info)
							{
								currentMethod->Expressions.push_back(ExpressionResult{
									(size_t)info->Type,
									info
								});
							}
						}
						if (token->TokenContextualType == ParserTokenContextualType::OpenBrace)
							elapsedOBraces++;
						else if (token->TokenContextualType == ParserTokenContextualType::ClosedBrace)
							elapsedCBraces++;
						if (elapsedCBraces == elapsedOBraces)
						{
							if (currentMethod)
								currentMethod = 0;
							if (currentClass)
								currentClass = 0;
						}
					} //scope
					break;
					}
					break;
					//end
				case PARSER_TOKEN_STATE_DIRECTIVE_USE:
					//begin
					if (token->TokenType != ParserTokenType::Identifier)
						ret->Errors.push_back(NewError("Expected an identifier", token->StartIndex, token->Length, false, currentInfo));
					else current->NamespacesUsed.push_back(token->StringValue);
					state = PARSER_TOKEN_STATE_NONE;
					break;
					//end
				case PARSER_TOKEN_STATE_DIRECTIVE_NMS:
					//begin
					if (token->TokenType != ParserTokenType::Identifier)
						ret->Errors.push_back(NewError("Expected an identifier", token->StartIndex, token->Length, false, currentInfo));
					else currentNms = token->StringValue;
					state = PARSER_TOKEN_STATE_NONE;
					break;
					//end
				case PARSER_TOKEN_STATE_IDENTIFIER:
					//begin
					if (token->ModifierValue)
					{
						currentAccessBuffer |= token->ModifierValue->Type;
						state = PARSER_TOKEN_STATE_NONE;
						break;
					}
					if (token->KeywordValue)
					{
						switch (token->KeywordValue->Type)
						{
						case KeywordType::Class:
							state = PARSER_TOKEN_STATE_CLASS;
							break;
						case KeywordType::Field:
							state = PARSER_TOKEN_STATE_FIELD;
							break;
						case KeywordType::Method:
							state = PARSER_TOKEN_STATE_METHOD;
							break;
						case KeywordType::Var:
							state = PARSER_TOKEN_STATE_VAR;
							break;
						}
						break;
					}
					if (currentMethod && (exprinfo = ParseExpression(token, &j, &tokens, &exprres)))
					{
						currentMethod->Expressions.push_back(ExpressionResult{
						(size_t)exprinfo->Type,
						exprres
						});
						switch (exprinfo->Type)
						{
						case ExpressionType::Depth:
							j--;
							break;
						case ExpressionType::Invocation:
						case ExpressionType::EIf:
							InvocationExprResult *invRes = (InvocationExprResult*)exprinfo->Expr;
							toIgnore = invRes->End;
							break;
						}
					}
					else {
						ret->Errors.push_back(NewError("Unknown identifier", token->StartIndex, token->Length, false, currentInfo));
					}
					state = PARSER_TOKEN_STATE_NONE;
					break;
					//end
				case PARSER_TOKEN_STATE_CLASS:
					//begin
					if (token->TokenType != ParserTokenType::Identifier)
					{
						ret->Errors.push_back(NewError("Expected an identifier", token->StartIndex, token->Length, false, currentInfo));
						state = PARSER_TOKEN_STATE_NONE;
						break;
					}
					if (j + 1 < tokens.size())
					{
						j++;
						ParserToken *next = &tokens[j];
						if (next->TokenType != ParserTokenType::Symbol)
						{
							ret->Errors.push_back(NewError("Expected symbol", next->StartIndex, next->Length, false, currentInfo));
						}
						else
						{
							_STD string prnt;
							if (next->TokenContextualType == ParserTokenContextualType::Colon)
							{
								if (j + 1 < tokens.size())
								{
									j++;
									next = &tokens[j];
									if (next->TokenType != ParserTokenType::Identifier)
									{
										ret->Errors.push_back(NewError("Expected an identifier", next->StartIndex, next->Length, false, currentInfo));
										state = PARSER_TOKEN_STATE_NONE;
										break;
									}
									prnt = next->StringValue;
								}
							}
							if (prnt.size() > 0)
							{
								if (j + 1 < tokens.size())
								{
									j++;
									next = &tokens[j];
								}
							}
							if (next->TokenContextualType != ParserTokenContextualType::OpenBrace)
							{
								ret->Errors.push_back(NewError("Expected '{'", next->StartIndex, next->Length, false, currentInfo));
								state = PARSER_TOKEN_STATE_NONE;
								break;
							}
							current->Classes.push_back(ClassToken{
								token->StringValue,
								prnt,
								currentNms
							});
							currentClass = &current->Classes.back();
						}
					}
					state = PARSER_TOKEN_STATE_NONE;
					break;
					//end
				case PARSER_TOKEN_STATE_FIELD:
					//begin
					if (token->TokenType != ParserTokenType::Identifier)
					{
						ret->Errors.push_back(NewError("Expected an identifier", token->StartIndex, token->Length, false, currentInfo));
						state = PARSER_TOKEN_STATE_NONE;
						break;
					}
					if (j + 1 < tokens.size())
					{
						j++;
						ParserToken *next = &tokens[j];
						if (next->TokenType != ParserTokenType::Identifier)
						{
							ret->Errors.push_back(NewError("Expected an identifier", next->StartIndex, next->Length, false, currentInfo));
						}
						else current->Fields.push_back(FieldToken{
							token,
							current->Classes.size() - 1,
							next->StringValue,
							currentAccessBuffer,
							token->StringValue
						});
						currentAccessBuffer = ModifierType::MNone;
						state = PARSER_TOKEN_STATE_NONE;
					}
					break;
					//end
				case PARSER_TOKEN_STATE_METHOD:
					//begin
					//async func int Life() {
					//
					//}
					if (token->TokenType != ParserTokenType::Identifier)
					{
						ret->Errors.push_back(NewError("Expected an identifier", token->StartIndex, token->Length, false, currentInfo));
						state = PARSER_TOKEN_STATE_NONE;
						break;
					}
					mtoken = MethodToken{
						token->StringValue
					};
					if (j + 1 < tokens.size())
					{
						j++;
						ParserToken *next = &tokens[j];
						if (next->TokenType != ParserTokenType::Identifier)
						{
							ret->Errors.push_back(NewError("Expected an identifier", next->StartIndex, next->Length, false, currentInfo));
							state = PARSER_TOKEN_STATE_NONE;
							break;
						}
						mtoken.Name = next->StringValue;
						if (j + 1 < tokens.size())
						{
							j++;
							next = &tokens[j];
							if (next->TokenType != ParserTokenType::Symbol)
							{
								ret->Errors.push_back(NewError("Expected symbol", next->StartIndex, next->Length, false, currentInfo));
								state = PARSER_TOKEN_STATE_NONE;
								break;
							}
							bool useP = false;
							if (next->TokenContextualType == ParserTokenContextualType::LeftParen)
							{
								useP = true;
								bool boutError = false;
								int pPos = 0;
								ParameterToken nextPToken;
								int ptokencount = 0;
								while (true)
								{
									if (j + 1 >= tokens.size()) {
										ret->Errors.push_back(NewError("Expected ')'", next->StartIndex, next->Length, false, currentInfo));
										state = PARSER_TOKEN_STATE_NONE;
										boutError = true;
										break;
									}
									j++;
									next = &tokens[j];
									if (next->TokenContextualType == ParserTokenContextualType::RightParen)
									{
										if (ptokencount % 2 != 0)
										{
											ret->Errors.push_back(NewError("Incomplete argument pair", next->StartIndex, next->Length, false, currentInfo));
											state = PARSER_TOKEN_STATE_NONE;
											boutError = true;
											break;
										}
										if (nextPToken.TypeName.size() > 0)
										{
											nextPToken.Pos = pPos;
											pPos++;
											mtoken.Parameters.push_back(nextPToken);
										}
										break;
									}
									if (next->TokenType != ParserTokenType::Identifier)
									{
										ret->Errors.push_back(NewError("Expected identifier", next->StartIndex, next->Length, false, currentInfo));
										state = PARSER_TOKEN_STATE_NONE;
										boutError = true;
										break;
									}
									if (ptokencount % 2 == 0)
										nextPToken.TypeName = next->StringValue;
									else
									{
										nextPToken.Name = next->StringValue;
										nextPToken.Pos = pPos;
										pPos++;
										mtoken.Parameters.push_back(nextPToken);
										nextPToken = ParameterToken();
									}
									ptokencount++;
								}
								if (boutError)
									continue;
							}
							if (useP)
							{
								if (j + 1 < tokens.size())
								{
									j++;
									next = &tokens[j];
								}
								else
								{
									ret->Errors.push_back(NewError("Expected '{'", next->StartIndex, next->Length, false, currentInfo));
									state = PARSER_TOKEN_STATE_NONE;
									break;
								}
							}
							if (next->TokenContextualType != ParserTokenContextualType::OpenBrace)
								ret->Errors.push_back(NewError("Expected '{'", next->StartIndex, next->Length, false, currentInfo));
							else
							{
								mtoken.ClassIdx = current->Classes.size() - 1;
								mtoken.Modifiers = currentAccessBuffer;
								currentAccessBuffer = ModifierType::MNone;
								current->Methods.push_back(mtoken);
								currentMethod = &current->Methods.back();
							}
							state = PARSER_TOKEN_STATE_NONE;
						}
					}
					break;
					//end
				case PARSER_TOKEN_STATE_METHOD_BODY:
					/*ExpressionInfo *expr = 0;
					void *exprRes = 0;
					while (expr = ParseExpression(token, &j, &tokens, &exprRes))
					{
						DepthExprResult *dres = 0;
						InvocationExprResult *ires = 0;
						switch (expr->Type)
						{
						case ExpressionType::Depth:
							dres = (DepthExprResult*)exprRes;
							_STD cout << "FOUND depth expr, parent: " << dres->Parent << ", child: " << dres->Child << "\n";
							token = dres->Start;
							break;
						case ExpressionType::Invocation:
							ires = (InvocationExprResult*)exprRes;
							_STD cout << "FOUND invocation expr, start: " << ires->Start << ", end: " << ires->End << "\n";
							SubVector<ParserToken> subv = SubVector<ParserToken>(ires->Start, ires->End, &tokens);
							break;
						}
					}*/
					break;
				case PARSER_TOKEN_STATE_VAR:
					if (!currentMethod)
					{
						ret->Errors.push_back(NewError("'var' can only exist inside a method context", token->StartIndex, token->Length, false, currentInfo));
						state = PARSER_TOKEN_STATE_NONE;
						break;
					}
					if (token->TokenType != ParserTokenType::Identifier)
					{
						ret->Errors.push_back(NewError("Expected an identifier", token->StartIndex, token->Length, false, currentInfo));
						state = PARSER_TOKEN_STATE_NONE;
						break;
					}
					if (j + 1 < tokens.size())
					{
						j++;
						ParserToken *next = &tokens[j];
						if (next->TokenType != ParserTokenType::Identifier)
						{
							ret->Errors.push_back(NewError("Expected an identifier", next->StartIndex, next->Length, false, currentInfo));
						}
						else currentMethod->Variables.push_back(VariableToken{
							token->StringValue,
							next->StringValue
						});
						state = PARSER_TOKEN_STATE_NONE;
					}
					break;
				}
			}
		}
		for (int i = 0; i < m_SrcFiles.size(); i++)
		{
			ParserSourceFile *current = &m_SrcFiles[i];
			for (int c = 0; c < current->Classes.size(); c++)
			{
				ClassToken *ctoken = &current->Classes[c];
				if (_CORE Registry::FindClass(ctoken->Name, ctoken->Namespaze))
				{
					//dup class
					ret->Errors.push_back(NewError("Class '" + ctoken->Namespaze + "::" + ctoken->Name + "' already exists",
						0, 0, false, &current->Info));
					continue;
				}
				_CORE Registry::RegisterClass(ctoken->Name, ctoken->Namespaze);
			}
		}
		for (int i = 0; i < m_SrcFiles.size(); i++)
		{
			ParserSourceFile *current = &m_SrcFiles[i];
			for (int f = 0; f < current->Fields.size(); f++)
			{
				FieldToken *ftoken = &current->Fields[f];
				ClassToken *ctoken = &current->Classes[ftoken->ClassIdx];
				_CLASSINFO *cinfo = 0;
				if (!(cinfo = _CORE Registry::FindClass(ctoken->Name, ctoken->Namespaze)))
				{
					ret->Errors.push_back(NewError("Class '" + ctoken->Namespaze + "::" + ctoken->Name + "' doesnt exist",
						0, 0, false, &current->Info));
					continue;
				}
				_CLASSINFO *_cinfo = SearchClass(ftoken->TypeString, current, cinfo);
				if (!_cinfo)
				{
					ret->Errors.push_back(NewError("Class '" + ftoken->TypeString + "' doesnt exist",
						0, 0, false, &current->Info));
					continue;
				}
				if (_CORE Registry::FindField(cinfo, ftoken->Name))
				{
					ret->Errors.push_back(NewError("Field '" + ctoken->Namespaze + "::" + ctoken->Name + "::" + ftoken->Name + "' already exists",
						0, 0, false, &current->Info));
					continue;
				}
				_CORE Registry::RegisterField(cinfo, ftoken->Name, _cinfo)->modifiers = ftoken->Modifiers;
			}
		}
		for (int i = 0; i < m_SrcFiles.size(); i++)
		{
			ParserSourceFile *current = &m_SrcFiles[i];
			for (int m = 0; m < current->Methods.size(); m++)
			{
				MethodToken *mtoken = &current->Methods[m];
				_CLASSINFO *cinfo = 0;
				ClassToken *ctoken = &current->Classes[mtoken->ClassIdx];
				if (!(cinfo = _CORE Registry::FindClass(ctoken->Name, ctoken->Namespaze)))
				{
					ret->Errors.push_back(NewError("Class '" + ctoken->Namespaze + "::" + ctoken->Name + "' doesnt exist",
						0, 0, false, &current->Info));
					continue;
				}
				_CLASSINFO *_cinfo = SearchClass(mtoken->TypeName, current, cinfo);
				if (!_cinfo)
				{
					ret->Errors.push_back(NewError("Class '" + mtoken->TypeName + "' doesnt exist",
						0, 0, false, &current->Info));
					continue;
				}
				if (_CORE Registry::FindMethod(cinfo, mtoken->Name, _CORE CalculateParamMagic(&mtoken->Parameters)))
				{
					_STD string pstr = "(";
					for (int p = 0; p < mtoken->Parameters.size(); p++)
					{
						pstr += mtoken->Parameters[p].TypeName;
						if (p < mtoken->Parameters.size() - 1)
							pstr += ',';
					}
					pstr += ')';
					ret->Errors.push_back(NewError("Method '" + ctoken->Namespaze + "::" + ctoken->Name + "::" + mtoken->Name + pstr + "' already exists",
						0, 0, false, &current->Info));
					continue;
				}
				bool regMethod = true;
				_STD vector<_PARAMETERINFO> pinfoVec;
				for (int p = 0; p < mtoken->Parameters.size(); p++)
				{
					ParameterToken *ptoken = &mtoken->Parameters[p];
					_CLASSINFO *pcinfo = SearchClass(ptoken->TypeName, current, cinfo);
					if (!pcinfo)
					{
						ret->Errors.push_back(NewError("Class '" + ptoken->TypeName + "' doesnt exist",
							0, 0, false, &current->Info));
						continue;
					}
					bool fbreak = false;
					for (int _p = 0; _p < mtoken->Parameters.size(); _p++)
					{
						if (_p == p)
							continue;
						if (mtoken->Parameters[_p].Name == ptoken->Name)
						{
							ret->Errors.push_back(NewError("Parameter '" + ptoken->Name + "' already exists",
								0, 0, false, &current->Info));
							fbreak = true;
							break;
						}
					}
					if (fbreak)
					{
						regMethod = false;
						break;
					}
					pinfoVec.push_back(ParameterInfo {
						strcpy(new char[ptoken->Name.size() + 1], ptoken->Name.c_str()),
						ptoken->Pos,
						pcinfo
					});
				}
				if (!regMethod)
					continue;
				_METHODINFO *minfo = _CORE Registry::RegisterMethod(cinfo, _cinfo, mtoken->Name, _CORE CalculateParamMagic(&mtoken->Parameters));
				minfo->modifiers = mtoken->Modifiers;
				for (int p = 0; p < pinfoVec.size(); p++)
				{
					Utils::AddArrayMember<_PARAMETERINFO>(minfo->parameters, minfo->parameter_count, pinfoVec[p]);
					minfo->parameter_count++;
				}
				for (int v = 0; v < mtoken->Variables.size(); v++)
				{
					VariableToken *vtoken = &mtoken->Variables[v];
					_CLASSINFO *vinfo = SearchClass(vtoken->TypeName, current, cinfo);
					if (!vinfo)
					{
						ret->Errors.push_back(NewError("Class '" + vtoken->TypeName + "' doesnt exist",
							0, 0, false, &current->Info));
						continue;
					}
					_CORE Registry::RegisterLocalVariable(minfo, vinfo, vtoken->Name);
				}
				for (int e = 0; e < mtoken->Expressions.size(); e++)
				{
					ExpressionResult *expr = &mtoken->Expressions[e];
					switch ((ExpressionType)expr->ExpressionType)
					{
					case ExpressionType::Depth:
						HandleDepthExpression(minfo, (DepthExprResult*)expr->Result, current, &ret->Errors);
						break;
					case ExpressionType::Invocation:
						HandleInvocationExpression(minfo, (InvocationExprResult*)expr->Result, current, &ret->Errors, &mtoken->Expressions, e);
						break;
					case ExpressionType::Arithmetic:
						Utils::AddArrayMember<ExpressionResult>(minfo->expressions, minfo->expression_count, ExpressionResult
						{
							ExpressionType::Arithmetic,
							expr->Result
						});
						minfo->expression_count++;
						break;
					}
				}
			}
		}
		ret->Img = _CORE Registry::SubmitImage(m_Libname ? m_Libname : "unknown");
		return ret;
	}

	ParserSourceFile *Parser::CurrentSourceFile()
	{
		return m_CurrentSourceFile;
	}

	ParserResult *Parser::ElapsedResult()
	{
		return m_CurrentResult;
	}

	void Parser::HandleInvocationExpression(_METHODINFO *method, InvocationExprResult *result, ParserSourceFile *current, _STD vector<ParserError> *errors, _STD vector<ExpressionResult> *expressions, int idx)
	{
		_METHODINFO *_method = 0;
		if ((*expressions)[idx].ExpressionType == ExpressionType::Depth)
		{
			ExpressionResult *nres = &method->expressions[method->expression_count - 1];
			if (nres->ExpressionType != ExpressionType::Depth)
			{
				errors->push_back(NewError("Unexpected error, nres->ExpressionType was not Depth", 0, 0, false, &current->Info));
				return;
			}
			CompilableDepthExpression *depth = (CompilableDepthExpression*)nres->Result;
			if (depth->Cx != 1)
			{
				errors->push_back(NewError(((_FIELDINFO*)depth->Child)->name + _STD string(" is not a method"), 0, 0, false, &current->Info));
				return;
			}
			_method = (_METHODINFO*)depth->Child;
		}
		else _method = Core::Registry::FindMethod(method->declaringType, result->PotentialMethodName, -1);
		if (!_method)
		{
			//cannot find method
			errors->push_back(NewError("Cannot find method '" + result->PotentialMethodName + "'", 0, 0, false, &current->Info));
			return;
		}
		ParserSourceFileInfo tmpSrc = ParserSourceFileInfo{
			result->ParamsString
		};
		_STD vector<ParserToken> tokens = CollectTokens(&tmpSrc);
		size_t firstIndex = _method->expression_count;
		for (int i = 0; i < tokens.size(); i++)
		{
			void *xresult;
			ExpressionInfo *exprinfo;
			while (exprinfo = ParseExpression(&tokens[i], &i, &tokens, &xresult))
			{
				_STD cout << "Param found for invocation, type: " << exprinfo->Type << ", method: " << result->PotentialMethodName << '\n';
				switch (exprinfo->Type)
				{
				case ExpressionType::Depth:
					HandleDepthExpression(method, (DepthExprResult*)xresult, current, errors);
					break;
				case ExpressionType::Invocation:
					HandleInvocationExpression(method, (InvocationExprResult*)xresult, current, errors, expressions, idx + 1);
					break;
				case ExpressionType::Arithmetic:
					Utils::AddArrayMember<ExpressionResult>(method->expressions, method->expression_count, ExpressionResult
					{
						ExpressionType::Arithmetic,
						xresult
					});
					method->expression_count++;
					break;
				}
				if (i >= tokens.size())
					break;
				i++;
			}
			i++;
		}
		Compiler::AddInstructionRegionBarrier(method->global_index, firstIndex, method->expression_count - firstIndex);
		Utils::AddArrayMember<ExpressionResult>(method->expressions, method->expression_count, ExpressionResult
		{
			ExpressionType::Invocation,
			new CompilableInvocationExpression{
			_method->global_index,
			firstIndex,
			method->expression_count
		}
		});
		method->expression_count++;
	}

	void Parser::HandleDepthExpression(_METHODINFO *method, DepthExprResult *result, ParserSourceFile *current, _STD vector<ParserError> *errors)
	{
		_CLASSINFO *parentClazz = 0;
		_FIELDINFO *localVar = 0;
		_FIELDINFO *localField = 0;
		void *parent = 0;
		short mx = -1;
		if (localVar = _CORE Registry::FindLocalVariable(method, result->Parent))
		{
			parentClazz = localVar->fieldType;
			parent = localVar;
			mx = 0;
		}
		else if (localField = _CORE Registry::FindField(method->declaringType, result->Parent))
		{
			parentClazz = localField->fieldType;
			parent = localField;
			mx = 1;
		}
		else if (!(parentClazz = SearchClass(result->Parent, current, method->declaringType)))
		{
			errors->push_back(NewError("'" + result->Parent + "' does not exist in the current context", result->Start->StartIndex, result->Start->Length,
				false, &current->Info));
			return;
		}
		else
		{
			parent = parentClazz;
			mx = 2;
		}
		short cx = -1;
		void *member = 0;
		if (member = _CORE Registry::FindField(parentClazz, result->Child))
			cx = 0;
		else if (member = _CORE Registry::FindMethod(parentClazz, result->Child, -1))
			cx = 1;
		else
		{
			errors->push_back(NewError("'" + result->Child + "' does not exist in '" + _STD string(parentClazz->name) + "'",
				result->Start->StartIndex, result->Start->Length, false, &current->Info));
			return;
		}
		Utils::AddArrayMember<ExpressionResult>(method->expressions, method->expression_count, ExpressionResult
		{
			ExpressionType::Depth,
			new CompilableDepthExpression {
				mx,
				cx,
				parent,
				member
			}
		});
		method->expression_count++;
	}
}