# This file contains a seperate implementation of the contraction process for grid graphs. Quite usefull to get an impression of how things behave. Outputs LaTeX (with TikZ).

class Graph
  def initialize dimension_x, dimension_y
    @size_x = Graph.grid_size(dimension_x)
    @size_y = Graph.grid_size(dimension_y)
    @nodes = Array.new(@size_x * @size_y) { [] }
    @contracted = []
    @size_x.times do |x|
      @size_y.times do |y|
        neighbours = []
        neighbours << coords_to_id(x + 1, y) if x + 1 < @size_x
        neighbours << coords_to_id(x, y + 1) if y + 1 < @size_y
        neighbours << coords_to_id(x - 1, y) if x > 0
        neighbours << coords_to_id(x, y - 1) if y > 0

        @nodes[coords_to_id x, y] += neighbours
      end
    end
  end

  def node_count
    return @size_x * @size_y
  end

  def add_edge from, to
    unless from == to || @nodes[from].include?(to)
      @nodes[from] << to
      @nodes[to] << from
    end
  end

  def remove node
    @nodes[node].each do |to|
      @nodes[to].delete node
    end
    @contracted[node] = true
  end

  def contract
    shortcuts = 0
    ordering = order
    @nodes.size.times do |i|
      current = ordering[i]
      yield current if block_given?

      remove current
      @nodes[current].each do |out_to|
        @nodes[current].each do |in_from|
          shortcuts += 1 if add_edge in_from, out_to
        end
      end
    end

    shortcuts
  end

  def order size_x = @size_x, size_y = @size_y
    return [] if size_x == 0 || size_y == 0
    return [0] if size_x == 1 && size_y == 1
    ordering = []
    size = { x: size_x, y: size_y }
    current = size_x >= size_y ? :x : :y
    other = -> (coord) { coord == :x ? :y : :x }
    offset = current == :x ? coords_to_id(size[current] / 2 + 1, 0) : coords_to_id(0, size[current] / 2 + 1)

    size[other.call(current)].times do |i|
      ordering.unshift current == :x ? coords_to_id(size[current] / 2, i) : coords_to_id(i, size[current] / 2)
    end

    size[current] /= 2
    suborder = order(size[:x], size[:y])
    ordering.unshift *suborder
    ordering.unshift *suborder.map { |id| id + offset }
    ordering
  end

  def coords_to_id x, y
    x * @size_y + y
  end

  def self.grid_size x
    2 ** x - 1
  end

  def to_tikz
    s = "\\begin{tikzpicture}\n"
    @size_x.times do |x|
      @size_y.times do |y|
        id = coords_to_id x, y
        s << "  \\node (#{id}) at (#{x}, #{y}) [node#{', deleted' if @contracted[id]}] {};\n" 
      end 
    end
    s << "  \\path[every node/.style={font=\\sffamily\\small}]\n"
    @nodes.each_with_index do |node, i|
      unless @contracted[i]
        node.each do |target|
          s << "  (#{i}) edge node {} (#{target})\n"
        end
      end
    end
    s << ";\n\\end{tikzpicture}\n\\\\"
  end
end

def visualize_contraction graph, output
  File.open output, 'w+' do |file|
    file.puts "\\documentclass{article}\n\\usepackage{tikz}\n\\usetikzlibrary{external}\n\\tikzexternalize\n\\begin{document}\n"

    graph.contract do |_|
      file.puts graph.to_tikz
    end

    file.puts "\\end{document}"
  end
end
