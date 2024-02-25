package com.github.jupj.jlox;

class AstPrinter implements Expr.Visitor<String>,
        Stmt.Visitor<String> {
    String print(Expr expr) {
        return expr.accept(this);
    }

    String print(Stmt stmt) {
        return stmt.accept(this);
    }

    @Override
    public String visitBlockStmt(Stmt.Block stmt) {
        StringBuilder builder = new StringBuilder();
        builder.append("(block");

        for (Stmt statement : stmt.statements) {
            builder.append(" ");
            builder.append(statement.accept(this));
        }

        builder.append(")");
        return builder.toString();
    }

    @Override
    public String visitClassStmt(Stmt.Class stmt) {
        StringBuilder builder = new StringBuilder();
        builder.append("(class " + stmt.name.lexeme);
        for (Stmt.Function method : stmt.methods) {
            builder.append(" " + function(method, "method"));
        }
        builder.append(")");
        return builder.toString();
    }

    @Override
    public String visitExpressionStmt(Stmt.Expression stmt) {
        return parenthesize(";", stmt.expression);
    }

    @Override
    public String visitFunctionStmt(Stmt.Function stmt) {
        return function(stmt, "fun");
    }

    @Override
    public String visitIfStmt(Stmt.If stmt) {
        if (stmt.elseBranch == null) {
            return parenthesize("if", stmt.condition, stmt.thenBranch);
        }

        return parenthesize("if-else", stmt.condition, stmt.thenBranch, stmt.elseBranch);
    }

    @Override
    public String visitPrintStmt(Stmt.Print stmt) {
        return parenthesize("print", stmt.expression);
    }

    @Override
    public String visitReturnStmt(Stmt.Return stmt) {
        return parenthesize("return", stmt.value);
    }

    @Override
    public String visitVarStmt(Stmt.Var stmt) {
        if (stmt.initializer == null) {
            return parenthesize("var", stmt.name);
        }

        return parenthesize("var", stmt.name, "=", stmt.initializer);
    }

    @Override
    public String visitWhileStmt(Stmt.While stmt) {
        return parenthesize("while", stmt.condition, stmt.body);
    }

    @Override
    public String visitAssignExpr(Expr.Assign expr) {
        return parenthesize("=",
                expr.name.lexeme, expr.value);
    }

    @Override
    public String visitBinaryExpr(Expr.Binary expr) {
        return parenthesize(expr.operator.lexeme,
                expr.left, expr.right);
    }

    @Override
    public String visitCallExpr(Expr.Call expr) {
        StringBuilder builder = new StringBuilder();
        builder.append("(call ");
        builder.append(expr.callee.accept(this));

        for (Expr argument : expr.arguments) {
            builder.append(argument.accept(this));
        }

        builder.append(")");
        return builder.toString();
    }

    @Override
    public String visitGetExpr(Expr.Get expr) {
        return parenthesize(".", expr.object, expr.name);
    }

    @Override
    public String visitGroupingExpr(Expr.Grouping expr) {
        return parenthesize("group", expr.expression);
    }

    @Override
    public String visitLiteralExpr(Expr.Literal expr) {
        if (expr.value == null)
            return "nil";
        return expr.value.toString();
    }

    @Override
    public String visitLogicalExpr(Expr.Logical expr) {
        return parenthesize(expr.operator.lexeme, expr.left, expr.right);
    }

    @Override
    public String visitSetExpr(Expr.Set expr) {
        return parenthesize("=",
                parenthesize(".", expr.object, expr.name),
                expr.value);
    }

    @Override
    public String visitThisExpr(Expr.This expr) {
        return expr.keyword.lexeme;
    }

    @Override
    public String visitUnaryExpr(Expr.Unary expr) {
        return parenthesize(expr.operator.lexeme, expr.right);
    }

    @Override
    public String visitVariableExpr(Expr.Variable expr) {
        return expr.name.lexeme;
    }

    private String parenthesize(String name, Object... parts) {
        StringBuilder builder = new StringBuilder();

        builder.append("(").append(name);
        for (Object part : parts) {
            builder.append(" ");
            if (part instanceof Expr) {
                builder.append(((Expr) part).accept(this));
            } else if (part instanceof Stmt) {
                builder.append(((Stmt) part).accept(this));
            } else if (part instanceof Token) {
                builder.append(((Token) part).lexeme);
            } else {
                builder.append(part);
            }
        }
        builder.append(")");

        return builder.toString();
    }

    private String function(Stmt.Function stmt, String type) {
        StringBuilder builder = new StringBuilder();
        builder.append("(" + type + " ");
        builder.append(stmt.name.lexeme);

        for (Token param : stmt.params) {
            builder.append(param.lexeme);
            builder.append(" ");
        }

        builder.append("(body");

        for (Stmt statement : stmt.body) {
            builder.append(" ");
            builder.append(statement.accept(this));
        }

        builder.append(")");
        builder.append(")");

        return builder.toString();
    }
}
