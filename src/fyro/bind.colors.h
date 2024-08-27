#pragma once

namespace lox
{
struct Lox;
}

namespace bind
{

void bind_named_colors(lox::Lox* lox);
void bind_rgb(lox::Lox* lox);

}  //  namespace bind
