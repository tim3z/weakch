# Converts a DIMACs .gr file to a METIS .graph file
#
# First argument: a graph in the DIMACs .gr format
#
# Writes a file named '${input}.graph' containing the same graph in the METIS .graph format. Any doubled edges will be removed.

unless File.exists? ARGV[0]
  puts 'File not found! Exiting!'
  exit
end


n_declared = m_declared = m_counted = 0
doubled_edges = 0
adjacency_array = []

puts 'reading file'

File.open(ARGV[0]).each do |line|
  type, source, target, weight = line.split(' ')
  case type
    when 'c' then
      next
    when 'p' then
      n_declared = target.to_i
      m_declared = weight.to_i
      puts "#{n_declared} vertices, #{m_declared} edges"
    when 'a'
      adjacency_array[source = source.to_i] ||= []
      if adjacency_array[source].include? target
        doubled_edges += 1
      else
        adjacency_array[source] << target
        m_counted += 1
      end
    else
      puts 'broken line'
  end
end
# adjacency_array.each_with_index do |adj, i|
#   puts "node #{i} has no outgoing arcs" if adj.nil? && i != 0
# end

adjacency_array = adjacency_array[1..-1]

puts "found less nodes then specified - only #{adjacency_array.compact.size} mentioned" if adjacency_array.compact.size != n_declared
puts "found #{doubled_edges} doubled edges" unless doubled_edges == 0
puts "found less unique edges then specified - only #{m_counted}" if m_counted != m_declared

puts 'exporting'

File.open("#{ARGV[0]}.graph", 'w+') do |export|
  export.puts "#{adjacency_array.size} #{m_counted / 2}"

  adjacency_array.each do |adjacent_nodes|
    if adjacent_nodes.nil?
      export.puts ''
    else
      export.puts adjacent_nodes.uniq.join(' ')
    end
  end
end

puts 'done - bye'