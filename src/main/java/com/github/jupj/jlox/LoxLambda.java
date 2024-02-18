package com.github.jupj.jlox;

import java.util.List;

class LoxLambda implements LoxCallable {
    private final Expr.Lambda expr;
    private final Environment closure;

    LoxLambda(Expr.Lambda expr, Environment closure) {
        this.closure = closure;
        this.expr = expr;
    }

    @Override
    public String toString() {
        return "<lambda>";
    }

    @Override
    public int arity() {
        return expr.params.size();
    }

    @Override
    public Object call(Interpreter interpreter,
            List<Object> arguments) {
        Environment environment = new Environment(closure);
        for (int i=0; i < expr.params.size(); i++) {
            environment.define(expr.params.get(i).lexeme, arguments.get(i));
        }

        try {
            interpreter.executeBlock(expr.body, environment);
        } catch (Return returnValue) {
            return returnValue.value;
        }
        return null;
    }
}
