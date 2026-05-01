#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_all.hpp>

#include <loomrv/ptl.hpp>
#include <loomrv/MTLEngine.hpp>
#include <loomrv/interval_set.hpp>

using namespace loomrv;
using namespace db_interval_set;

// Recursive structural tree comparison (from src/test_parse.cpp)
static bool trees_equal(const std::vector<DenseNode>& a, int rootA,
                        const std::vector<DenseNode>& b, int rootB) {
    const auto& na = a[rootA];
    const auto& nb = b[rootB];

    if (na.type != nb.type || na.a != nb.a || na.b != nb.b)
        return false;

    if (na.type == NodeType::PROPOSITION)
        return true;

    // Unary operators (NOT, EVENTUALLY, ALWAYS): only right child matters
    if (na.type == NodeType::NOT || na.type == NodeType::EVENTUALLY || na.type == NodeType::ALWAYS)
        return trees_equal(a, na.rightOperandIndex, b, nb.rightOperandIndex);

    // Binary operators (IMPLIES, SINCE): both children must match
    if (na.type == NodeType::IMPLIES || na.type == NodeType::SINCE) {
        return trees_equal(a, na.leftOperandIndex, b, nb.leftOperandIndex)
            && trees_equal(a, na.rightOperandIndex, b, nb.rightOperandIndex);
    }
    // Commutative Binary operators (AND, OR): both children must match, but order can change.
    return (trees_equal(a, na.leftOperandIndex, b, nb.leftOperandIndex)
        && trees_equal(a, na.rightOperandIndex, b, nb.rightOperandIndex))
        ||
        (trees_equal(a, na.leftOperandIndex, b, nb.rightOperandIndex)
        && trees_equal(a, na.rightOperandIndex, b, nb.leftOperandIndex));
}


TEST_CASE("AbsentAQ parse", "[parse]") {
    // historically((once[:10]{q}) -> ((not{p}) since {q}))
    std::string formula = "historically((once[:10]{q}) -> ((not{p}) since {q}))";

    ptl_parser p;
    auto monitor = createDenseMultiPropertyMonitor(1000);
    p.parse_dense(formula, monitor);
    auto& parsed = monitor.nodes;

    IntervalSetHolder holder = newHolder(1000);
    DenseNode q{empty(holder), empty(holder), NodeType::PROPOSITION, 0, 0, 0, 0};
    DenseNode p_node{empty(holder), empty(holder), NodeType::PROPOSITION, 1, 0, 0, 0};
    DenseNode once{empty(holder), empty(holder), NodeType::EVENTUALLY, 0, 0, 0, 10};
    DenseNode notNode{empty(holder), empty(holder), NodeType::NOT, 0, 1, 0, 0};
    DenseNode since{empty(holder), empty(holder), NodeType::SINCE, 3, 0, 0, B_INFINITY};
    DenseNode implies{empty(holder), empty(holder), NodeType::IMPLIES, 2, 4, 0, 0};
    DenseNode always{empty(holder), empty(holder), NodeType::ALWAYS, 0, 5, 0, B_INFINITY};
    std::vector<DenseNode> expected{q, p_node, once, notNode, since, implies, always};

    REQUIRE(trees_equal(parsed, parsed.size() - 1, expected, expected.size() - 1));
    destroyHolder(holder);
}

TEST_CASE("AbsentBQR parse", "[parse]") {
    // historically(({r} && !{q} && once{q}) -> ((not{p}) since[3:10] {q}))
    // Note: corrected parenthesization so historically wraps the full implication
    std::string formula = "historically(({r} && !{q} && once{q}) -> ((not{p}) since[3:10] {q}))";

    ptl_parser p;
    auto monitor = createDenseMultiPropertyMonitor(1000);
    p.parse_dense(formula, monitor);
    auto& parsed = monitor.nodes;

    IntervalSetHolder holder = newHolder(1000);
    DenseNode q{empty(holder), empty(holder), NodeType::PROPOSITION, 0, 0, 0, 0};
    DenseNode p_node{empty(holder), empty(holder), NodeType::PROPOSITION, 1, 0, 0, 0};
    DenseNode r{empty(holder), empty(holder), NodeType::PROPOSITION, 2, 0, 0, 0};
    DenseNode not_q{empty(holder), empty(holder), NodeType::NOT, 0, 0, 0, 0};
    DenseNode once_q{empty(holder), empty(holder), NodeType::EVENTUALLY, 0, 0, 0, B_INFINITY};
    DenseNode and1{empty(holder), empty(holder), NodeType::AND, 2, 3, 0, 0};
    DenseNode and2{empty(holder), empty(holder), NodeType::AND, 5, 4, 0, 0};
    DenseNode not_p{empty(holder), empty(holder), NodeType::NOT, 0, 1, 0, 0};
    DenseNode since_node{empty(holder), empty(holder), NodeType::SINCE, 7, 0, 3, 10};
    DenseNode implies_node{empty(holder), empty(holder), NodeType::IMPLIES, 6, 8, 0, 0};
    DenseNode always_node{empty(holder), empty(holder), NodeType::ALWAYS, 0, 9, 0, B_INFINITY};
    std::vector<DenseNode> expected{q, p_node, r, not_q, once_q, and1, and2, not_p, since_node, implies_node, always_node};

    REQUIRE(trees_equal(parsed, parsed.size() - 1, expected, expected.size() - 1));
    destroyHolder(holder);
}

TEST_CASE("AbsentBR parse", "[parse]") {
    // historically({r} -> (historically[:10](not{p})))
    std::string formula = "historically({r} -> (historically[:10](not{p})))";

    ptl_parser p;
    auto monitor = createDenseMultiPropertyMonitor(1000);
    p.parse_dense(formula, monitor);
    auto& parsed = monitor.nodes;

    IntervalSetHolder holder = newHolder(1000);
    DenseNode q{empty(holder), empty(holder), NodeType::PROPOSITION, 0, 0, 0, 0};
    DenseNode p_node{empty(holder), empty(holder), NodeType::PROPOSITION, 1, 0, 0, 0};
    DenseNode r{empty(holder), empty(holder), NodeType::PROPOSITION, 2, 0, 0, 0};
    DenseNode not_p{empty(holder), empty(holder), NodeType::NOT, 0, 1, 0, 0};
    DenseNode inner_always{empty(holder), empty(holder), NodeType::ALWAYS, 0, 3, 0, 10};
    DenseNode implies_node{empty(holder), empty(holder), NodeType::IMPLIES, 2, 4, 0, 0};
    DenseNode root_always{empty(holder), empty(holder), NodeType::ALWAYS, 0, 5, 0, B_INFINITY};
    std::vector<DenseNode> expected{q, p_node, r, not_p, inner_always, implies_node, root_always};

    REQUIRE(trees_equal(parsed, parsed.size() - 1, expected, expected.size() - 1));
    destroyHolder(holder);
}

TEST_CASE("AlwaysAQ parse", "[parse]") {
    // historically((once[:10]{q}) -> ({p} since {q}))
    std::string formula = "historically((once[:10]{q}) -> ({p} since {q}))";

    ptl_parser p;
    auto monitor = createDenseMultiPropertyMonitor(1000);
    p.parse_dense(formula, monitor);
    auto& parsed = monitor.nodes;

    IntervalSetHolder holder = newHolder(1000);
    DenseNode q{empty(holder), empty(holder), NodeType::PROPOSITION, 0, 0, 0, 0};
    DenseNode p_node{empty(holder), empty(holder), NodeType::PROPOSITION, 1, 0, 0, 0};
    DenseNode r{empty(holder), empty(holder), NodeType::PROPOSITION, 2, 0, 0, 0};
    DenseNode once_q{empty(holder), empty(holder), NodeType::EVENTUALLY, 0, 0, 0, 10};
    DenseNode since_node{empty(holder), empty(holder), NodeType::SINCE, 1, 0, 0, B_INFINITY};
    DenseNode implies_node{empty(holder), empty(holder), NodeType::IMPLIES, 3, 4, 0, 0};
    DenseNode root_always{empty(holder), empty(holder), NodeType::ALWAYS, 0, 5, 0, B_INFINITY};
    std::vector<DenseNode> expected{q, p_node, r, once_q, since_node, implies_node, root_always};

    REQUIRE(trees_equal(parsed, parsed.size() - 1, expected, expected.size() - 1));
    destroyHolder(holder);
}

TEST_CASE("AlwaysBQR parse", "[parse]") {
    // historically(({r} && !{q} && once{q}) -> ({p} since[3:10] {q}))
    std::string formula = "historically(({r} && !{q} && once{q}) -> ({p} since[3:10] {q}))";

    ptl_parser p;
    auto monitor = createDenseMultiPropertyMonitor(1000);
    p.parse_dense(formula, monitor);
    auto& parsed = monitor.nodes;

    IntervalSetHolder holder = newHolder(1000);
    DenseNode q{empty(holder), empty(holder), NodeType::PROPOSITION, 0, 0, 0, 0};
    DenseNode p_node{empty(holder), empty(holder), NodeType::PROPOSITION, 1, 0, 0, 0};
    DenseNode r{empty(holder), empty(holder), NodeType::PROPOSITION, 2, 0, 0, 0};
    DenseNode not_q{empty(holder), empty(holder), NodeType::NOT, 0, 0, 0, 0};
    DenseNode once_q{empty(holder), empty(holder), NodeType::EVENTUALLY, 0, 0, 0, B_INFINITY};
    DenseNode and1{empty(holder), empty(holder), NodeType::AND, 2, 3, 0, 0};
    DenseNode and2{empty(holder), empty(holder), NodeType::AND, 5, 4, 0, 0};
    DenseNode not_p{empty(holder), empty(holder), NodeType::NOT, 0, 1, 0, 0};
    DenseNode since_node{empty(holder), empty(holder), NodeType::SINCE, 1, 0, 3, 10};
    DenseNode implies_node{empty(holder), empty(holder), NodeType::IMPLIES, 6, 8, 0, 0};
    DenseNode always_node{empty(holder), empty(holder), NodeType::ALWAYS, 0, 9, 0, B_INFINITY};
    std::vector<DenseNode> expected{q, p_node, r, not_q, once_q, and1, and2, not_p, since_node, implies_node, always_node};

    REQUIRE(trees_equal(parsed, parsed.size() - 1, expected, expected.size() - 1));
    destroyHolder(holder);
}

TEST_CASE("AlwaysBR parse", "[parse]") {
    // historically({r} -> (historically[:10]{p}))
    std::string formula = "historically({r} -> (historically[:10]{p}))";

    ptl_parser p;
    auto monitor = createDenseMultiPropertyMonitor(1000);
    p.parse_dense(formula, monitor);
    auto& parsed = monitor.nodes;

    IntervalSetHolder holder = newHolder(1000);
    DenseNode q{empty(holder), empty(holder), NodeType::PROPOSITION, 0, 0, 0, 0};
    DenseNode p_node{empty(holder), empty(holder), NodeType::PROPOSITION, 1, 0, 0, 0};
    DenseNode r{empty(holder), empty(holder), NodeType::PROPOSITION, 2, 0, 0, 0};
    DenseNode inner_always{empty(holder), empty(holder), NodeType::ALWAYS, 0, 1, 0, 10};
    DenseNode implies_node{empty(holder), empty(holder), NodeType::IMPLIES, 2, 3, 0, 0};
    DenseNode root_always{empty(holder), empty(holder), NodeType::ALWAYS, 0, 4, 0, B_INFINITY};
    std::vector<DenseNode> expected{q, p_node, r, inner_always, implies_node, root_always};

    REQUIRE(trees_equal(parsed, parsed.size() - 1, expected, expected.size() - 1));
    destroyHolder(holder);
}

TEST_CASE("RecurBQR parse", "[parse]") {
    // historically(({r} && !{q} && once{q}) -> ((once[:10]({p} or {q})) since {q}))
    std::string formula = "historically(({r} && !{q} && once{q}) -> ((once[:10]({p} or {q})) since {q}))";

    ptl_parser p;
    auto monitor = createDenseMultiPropertyMonitor(1000);
    p.parse_dense(formula, monitor);
    auto& parsed = monitor.nodes;

    IntervalSetHolder holder = newHolder(1000);
    DenseNode q{empty(holder), empty(holder), NodeType::PROPOSITION, 0, 0, 0, 0};
    DenseNode p_node{empty(holder), empty(holder), NodeType::PROPOSITION, 1, 0, 0, 0};
    DenseNode r{empty(holder), empty(holder), NodeType::PROPOSITION, 2, 0, 0, 0};
    DenseNode not_q{empty(holder), empty(holder), NodeType::NOT, 0, 0, 0, 0};
    DenseNode once_q{empty(holder), empty(holder), NodeType::EVENTUALLY, 0, 0, 0, B_INFINITY};
    DenseNode and1{empty(holder), empty(holder), NodeType::AND, 2, 3, 0, 0};
    DenseNode and2{empty(holder), empty(holder), NodeType::AND, 5, 4, 0, 0};
    DenseNode p_or_q{empty(holder), empty(holder), NodeType::OR, 1, 0, 0, 0};
    DenseNode once_p_or_q{empty(holder), empty(holder), NodeType::EVENTUALLY, 0, 7, 0, 10};
    DenseNode since_node{empty(holder), empty(holder), NodeType::SINCE, 8, 0, 0, B_INFINITY};
    DenseNode implies_node{empty(holder), empty(holder), NodeType::IMPLIES, 6, 9, 0, 0};
    DenseNode root_always{empty(holder), empty(holder), NodeType::ALWAYS, 0, 10, 0, B_INFINITY};
    std::vector<DenseNode> expected{q, p_node, r, not_q, once_q, and1, and2, p_or_q, once_p_or_q, since_node, implies_node, root_always};

    REQUIRE(trees_equal(parsed, parsed.size() - 1, expected, expected.size() - 1));
    destroyHolder(holder);
}

TEST_CASE("RecurGLB parse", "[parse]") {
    // historically(once[:10]{p})
    std::string formula = "historically(once[:10]{p})";

    ptl_parser p;
    auto monitor = createDenseMultiPropertyMonitor(1000);
    p.parse_dense(formula, monitor);
    auto& parsed = monitor.nodes;

    IntervalSetHolder holder = newHolder(1000);
    DenseNode p_node{empty(holder), empty(holder), NodeType::PROPOSITION, 0, 0, 0, 0};
    DenseNode once{empty(holder), empty(holder), NodeType::EVENTUALLY, 0, 0, 0, 10};
    DenseNode always{empty(holder), empty(holder), NodeType::ALWAYS, 0, 1, 0, B_INFINITY};
    std::vector<DenseNode> expected{p_node, once, always};

    REQUIRE(trees_equal(parsed, parsed.size() - 1, expected, expected.size() - 1));
    destroyHolder(holder);
}

TEST_CASE("RespondBQR parse", "[parse]") {
    // historically(({r} && !{q} && once{q}) -> ( (({s} -> once[3:10]{p}) and not((not {s}) since[10:] {p})) since {q}))
    std::string formula = "historically(({r} && !{q} && once{q}) -> ( (({s} -> once[3:10]{p}) and not((not {s}) since[10:] {p})) since {q}))";

    ptl_parser p;
    auto monitor = createDenseMultiPropertyMonitor(1000);
    p.parse_dense(formula, monitor);
    auto& parsed = monitor.nodes;

    IntervalSetHolder holder = newHolder(1000);
    DenseNode q{empty(holder), empty(holder), NodeType::PROPOSITION, 0, 0, 0, 0};
    DenseNode p_node{empty(holder), empty(holder), NodeType::PROPOSITION, 1, 0, 0, 0};
    DenseNode s{empty(holder), empty(holder), NodeType::PROPOSITION, 2, 0, 0, 0};
    DenseNode r{empty(holder), empty(holder), NodeType::PROPOSITION, 3, 0, 0, 0};
    DenseNode not_q{empty(holder), empty(holder), NodeType::NOT, 0, 0, 0, 0};
    DenseNode once_q{empty(holder), empty(holder), NodeType::EVENTUALLY, 0, 0, 0, B_INFINITY};
    DenseNode and_A1{empty(holder), empty(holder), NodeType::AND, 3, 4, 0, 0};
    DenseNode and_A2{empty(holder), empty(holder), NodeType::AND, 6, 5, 0, 0};
    DenseNode once_p{empty(holder), empty(holder), NodeType::EVENTUALLY, 0, 1, 3, 10};
    DenseNode implies_D{empty(holder), empty(holder), NodeType::IMPLIES, 2, 8, 0, 0};
    DenseNode not_s{empty(holder), empty(holder), NodeType::NOT, 0, 2, 0, 0};
    DenseNode since_F{empty(holder), empty(holder), NodeType::SINCE, 10, 1, 10, B_INFINITY};
    DenseNode not_F{empty(holder), empty(holder), NodeType::NOT, 0, 11, 0, 0};
    DenseNode and_C{empty(holder), empty(holder), NodeType::AND, 9, 12, 0, 0};
    DenseNode since_B{empty(holder), empty(holder), NodeType::SINCE, 13, 0, 0, B_INFINITY};
    DenseNode implies_main{empty(holder), empty(holder), NodeType::IMPLIES, 7, 14, 0, 0};
    DenseNode root_always{empty(holder), empty(holder), NodeType::ALWAYS, 0, 15, 0, B_INFINITY};
    std::vector<DenseNode> expected{q, p_node, s, r, not_q, once_q, and_A1, and_A2, once_p, implies_D, not_s, since_F, not_F, and_C, since_B, implies_main, root_always};

    REQUIRE(trees_equal(parsed, parsed.size() - 1, expected, expected.size() - 1));
    destroyHolder(holder);
}

TEST_CASE("RespondGLB parse", "[parse]") {
    // historically(({s} -> once[3:10]{p}) and not((not {s}) since[10:] {p}))
    std::string formula = "historically(({s} -> once[3:10]{p}) and not((not {s}) since[10:] {p}))";

    ptl_parser p;
    auto monitor = createDenseMultiPropertyMonitor(1000);
    p.parse_dense(formula, monitor);
    auto& parsed = monitor.nodes;

    IntervalSetHolder holder = newHolder(1000);
    DenseNode p_node{empty(holder), empty(holder), NodeType::PROPOSITION, 0, 0, 0, 0};
    DenseNode s{empty(holder), empty(holder), NodeType::PROPOSITION, 1, 0, 0, 0};
    DenseNode once_p{empty(holder), empty(holder), NodeType::EVENTUALLY, 0, 0, 3, 10};
    DenseNode implies_D{empty(holder), empty(holder), NodeType::IMPLIES, 1, 2, 0, 0};
    DenseNode not_s{empty(holder), empty(holder), NodeType::NOT, 0, 1, 0, 0};
    DenseNode since_F{empty(holder), empty(holder), NodeType::SINCE, 4, 0, 10, B_INFINITY};
    DenseNode not_F{empty(holder), empty(holder), NodeType::NOT, 0, 5, 0, 0};
    DenseNode and_C{empty(holder), empty(holder), NodeType::AND, 3, 6, 0, 0};
    DenseNode root_always{empty(holder), empty(holder), NodeType::ALWAYS, 0, 7, 0, B_INFINITY};
    std::vector<DenseNode> expected{p_node, s, once_p, implies_D, not_s, since_F, not_F, and_C, root_always};

    REQUIRE(trees_equal(parsed, parsed.size() - 1, expected, expected.size() - 1));
    destroyHolder(holder);
}

TEST_CASE("Parse with different bound values", "[parse]") {
    ptl_parser p;

    SECTION("UpperBound [:100]") {
        std::string formula = "historically((once[:100]{q}) -> ((not{p}) since {q}))";
        auto monitor = createDenseMultiPropertyMonitor(1000);
        p.parse_dense(formula, monitor);
        auto& parsed = monitor.nodes;

        IntervalSetHolder holder = newHolder(1000);
        DenseNode q{empty(holder), empty(holder), NodeType::PROPOSITION, 0, 0, 0, 0};
        DenseNode p_node{empty(holder), empty(holder), NodeType::PROPOSITION, 1, 0, 0, 0};
        DenseNode once{empty(holder), empty(holder), NodeType::EVENTUALLY, 0, 0, 0, 100};
        DenseNode notNode{empty(holder), empty(holder), NodeType::NOT, 0, 1, 0, 0};
        DenseNode since{empty(holder), empty(holder), NodeType::SINCE, 3, 0, 0, B_INFINITY};
        DenseNode implies{empty(holder), empty(holder), NodeType::IMPLIES, 2, 4, 0, 0};
        DenseNode always{empty(holder), empty(holder), NodeType::ALWAYS, 0, 5, 0, B_INFINITY};
        std::vector<DenseNode> expected{q, p_node, once, notNode, since, implies, always};

        REQUIRE(trees_equal(parsed, parsed.size() - 1, expected, expected.size() - 1));
        destroyHolder(holder);
    }

    SECTION("FullBound [30:100]") {
        std::string formula = "historically(({r} && !{q} && once{q}) -> ({p} since[30:100] {q}))";
        auto monitor = createDenseMultiPropertyMonitor(1000);
        p.parse_dense(formula, monitor);
        auto& parsed = monitor.nodes;

        IntervalSetHolder holder = newHolder(1000);
        DenseNode q{empty(holder), empty(holder), NodeType::PROPOSITION, 0, 0, 0, 0};
        DenseNode p_node{empty(holder), empty(holder), NodeType::PROPOSITION, 1, 0, 0, 0};
        DenseNode r{empty(holder), empty(holder), NodeType::PROPOSITION, 2, 0, 0, 0};
        DenseNode not_q{empty(holder), empty(holder), NodeType::NOT, 0, 0, 0, 0};
        DenseNode once_q{empty(holder), empty(holder), NodeType::EVENTUALLY, 0, 0, 0, B_INFINITY};
        DenseNode and1{empty(holder), empty(holder), NodeType::AND, 2, 3, 0, 0};
        DenseNode and2{empty(holder), empty(holder), NodeType::AND, 5, 4, 0, 0};
        DenseNode not_p{empty(holder), empty(holder), NodeType::NOT, 0, 1, 0, 0};
        DenseNode since_node{empty(holder), empty(holder), NodeType::SINCE, 1, 0, 30, 100};
        DenseNode implies_node{empty(holder), empty(holder), NodeType::IMPLIES, 6, 8, 0, 0};
        DenseNode always_node{empty(holder), empty(holder), NodeType::ALWAYS, 0, 9, 0, B_INFINITY};
        std::vector<DenseNode> expected{q, p_node, r, not_q, once_q, and1, and2, not_p, since_node, implies_node, always_node};

        REQUIRE(trees_equal(parsed, parsed.size() - 1, expected, expected.size() - 1));
        destroyHolder(holder);
    }

    SECTION("LowerBound [10:]") {
        std::string formula = "historically(({s} -> once[3:10]{p}) and not((not {s}) since[10:] {p}))";
        auto monitor = createDenseMultiPropertyMonitor(1000);
        p.parse_dense(formula, monitor);
        auto& parsed = monitor.nodes;

        // Verify the since node has a=10, b=B_INFINITY (lower bound)
        // Find the SINCE node with a=10
        bool found_lower_bound_since = false;
        for (auto& node : parsed) {
            if (node.type == NodeType::SINCE && node.a == 10 && node.b == B_INFINITY) {
                found_lower_bound_since = true;
                break;
            }
        }
        REQUIRE(found_lower_bound_since);
    }
}

TEST_CASE("Parse invalid formula", "[parse]") {
    ptl_parser p;
    auto monitor = createDenseMultiPropertyMonitor(1000);

    REQUIRE_THROWS_AS(p.parse_dense("historically(invalid syntax here", monitor), std::runtime_error);
}

TEST_CASE("Proposition map correctness", "[parse]") {
    ptl_parser p;

    SECTION("Two propositions") {
        auto monitor = createDenseMultiPropertyMonitor(1000);
        p.parse_dense("historically((once[:10]{q}) -> ((not{p}) since {q}))", monitor);

        REQUIRE(monitor.proposition_map.size() == 2);
        REQUIRE(monitor.proposition_map.count("q") == 1);
        REQUIRE(monitor.proposition_map.count("p") == 1);
    }

    SECTION("Three propositions") {
        auto monitor = createDenseMultiPropertyMonitor(1000);
        p.parse_dense("historically({r} -> (historically[:10](not{p})))", monitor);

        REQUIRE(monitor.proposition_map.size() == 2);
        REQUIRE(monitor.proposition_map.count("r") == 1);
        REQUIRE(monitor.proposition_map.count("p") == 1);
    }

    SECTION("Four propositions") {
        auto monitor = createDenseMultiPropertyMonitor(1000);
        p.parse_dense("historically(({r} && !{q} && once{q}) -> ( (({s} -> once[3:10]{p}) and not((not {s}) since[10:] {p})) since {q}))", monitor);

        REQUIRE(monitor.proposition_map.size() == 4);
        REQUIRE(monitor.proposition_map.count("r") == 1);
        REQUIRE(monitor.proposition_map.count("q") == 1);
        REQUIRE(monitor.proposition_map.count("s") == 1);
        REQUIRE(monitor.proposition_map.count("p") == 1);
    }
}

TEST_CASE("Node count correctness", "[parse]") {
    ptl_parser p;

    SECTION("RecurGLB - 3 nodes") {
        auto monitor = createDenseMultiPropertyMonitor(1000);
        p.parse_dense("historically(once[:10]{p})", monitor);
        REQUIRE(monitor.nodes.size() == 3);
    }

    SECTION("AbsentAQ - 7 nodes") {
        auto monitor = createDenseMultiPropertyMonitor(1000);
        p.parse_dense("historically((once[:10]{q}) -> ((not{p}) since {q}))", monitor);
        REQUIRE(monitor.nodes.size() == 7);
    }

    SECTION("RespondBQR - 17 nodes") {
        auto monitor = createDenseMultiPropertyMonitor(1000);
        p.parse_dense("historically(({r} && !{q} && once{q}) -> ( (({s} -> once[3:10]{p}) and not((not {s}) since[10:] {p})) since {q}))", monitor);
        REQUIRE(monitor.nodes.size() == 17);
    }
}
