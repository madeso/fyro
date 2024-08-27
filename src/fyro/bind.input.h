#pragma once

namespace lox
{
	struct Lox;
}
struct Input;

namespace bind
{

void bind_fun_get_input(lox::Lox* lox, Input* input);
void bind_player(lox::Lox* lox);
void bind_input_frame(lox::Lox* lox);

}
