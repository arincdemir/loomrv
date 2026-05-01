#include <iostream>
#include <loomrv/ptl.hpp>
#include <loomrv/MTLEngine.hpp>
#include <loomrv/interval_set.hpp>

using namespace loomrv;
using namespace db_interval_set;

std::vector<DenseNode> get_hand_parsed (db_interval_set::IntervalSetHolder holder) {
        
    DenseNode q{empty(holder), empty(holder), NodeType::PROPOSITION, 0, 0, 0, 0};
    DenseNode p{empty(holder), empty(holder), NodeType::PROPOSITION, 1, 0, 0, 0};
    DenseNode r{empty(holder), empty(holder), NodeType::PROPOSITION, 2, 0, 0, 0};
    DenseNode not_q{empty(holder), empty(holder), NodeType::NOT, 0, 0, 0, 0};
    DenseNode once_q{empty(holder), empty(holder), NodeType::EVENTUALLY, 0, 0, 0, B_INFINITY};
    DenseNode and1{empty(holder), empty(holder), NodeType::AND, 2, 3, 0, 0};
    DenseNode and2{empty(holder), empty(holder), NodeType::AND, 5, 4, 0, 0};
    DenseNode p_or_q{empty(holder), empty(holder), NodeType::OR, 1, 0, 0, 0};
    DenseNode once_p_or_q{empty(holder), empty(holder), NodeType::EVENTUALLY, 0, 7, 0, 1000};
    DenseNode since_node{empty(holder), empty(holder), NodeType::SINCE, 8, 0, 0, B_INFINITY};
    DenseNode implies_node{empty(holder), empty(holder), NodeType::IMPLIES, 6, 9, 0, 0};
    DenseNode root_always{empty(holder), empty(holder), NodeType::ALWAYS, 0, 10, 0, B_INFINITY};
    std::vector<DenseNode> nodes{q, p, r, not_q, once_q, and1, and2, p_or_q, once_p_or_q, since_node, implies_node, root_always};
    return nodes;
}


bool trees_equal(const std::vector<DenseNode>& a, int rootA,
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
    
    // Binary operators (AND, OR, IMPLIES, SINCE): both children must match
    return trees_equal(a, na.leftOperandIndex, b, nb.leftOperandIndex)
        && trees_equal(a, na.rightOperandIndex, b, nb.rightOperandIndex);
}

int main() {
    ptl_parser p;

    std::string formula = "historically(({r} && !{q} && once{q}) -> ((once[:1000]({p} or {q})) since {q}))";

    std::cout << "Parsing: " << formula << std::endl;

    try {

        db_interval_set::IntervalSetHolder handCraftedHolder = db_interval_set::newHolder(1000);
        auto handCraftedNodes = get_hand_parsed(handCraftedHolder);

        // Use the new DenseMultiPropertyMonitor
        auto monitor = createDenseMultiPropertyMonitor(1000);
        p.parse_dense(formula, monitor);

        auto &nodes = monitor.nodes;
        std::cout << "Parsed successfully! " << nodes.size() << " nodes created." << std::endl;
        std::cout << "Property count: " << monitor.propertyCount << std::endl;

        // Show proposition map
        std::cout << "\nProposition map:" << std::endl;
        for (auto &[name, idx] : monitor.proposition_map) {
            std::cout << "  " << name << " -> node index " << idx << std::endl;
        }
        std::cout << std::endl;

        for (size_t i = 0; i < nodes.size(); i++) {
            std::cout << "Node " << i << ": type=";
            switch (nodes[i].type) {
                case NodeType::PROPOSITION: std::cout << "PROPOSITION"; break;
                case NodeType::AND:         std::cout << "AND"; break;
                case NodeType::OR:          std::cout << "OR"; break;
                case NodeType::NOT:         std::cout << "NOT"; break;
                case NodeType::IMPLIES:     std::cout << "IMPLIES"; break;
                case NodeType::EVENTUALLY:  std::cout << "EVENTUALLY"; break;
                case NodeType::ALWAYS:      std::cout << "ALWAYS"; break;
                case NodeType::SINCE:       std::cout << "SINCE"; break;
                case NodeType::TEST:        std::cout << "TEST"; break;
            }
            std::cout << " left=" << nodes[i].leftOperandIndex
                      << " right=" << nodes[i].rightOperandIndex
                      << " a=" << nodes[i].a
                      << " b=" << nodes[i].b
                      << std::endl;
        }

        std::cout << "two trees are equal = " << trees_equal(nodes, nodes.size() - 1,
         handCraftedNodes, handCraftedNodes.size() - 1) << std::endl;
        
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }


    return 0;
}
