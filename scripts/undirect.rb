# Adds missing backward arcs to a graph for METIS
#
# First argument: a graph in the METIS .graph format
#
# Writes a file named 'undirected.${input}' containing the given graph in the METIS .graph format where for each arc there also exists the backward arc.

unless File.exists? ARGV[0]
  puts 'File not found! Exiting!'
  exit
end

puts 'reading file'
adjacency_array = []
enumerator = File.open(ARGV[0]).each
nodes, _ = enumerator.next.split(' ')
adjacency_array = Array.new(nodes.to_i) { [] }

i = -1
enumerator.each do |line|
  adjacency_array[i += 1] = line.split(' ').map { |id| id.to_i }
end
puts 'done reading'

puts 'adding missing edges'
added = 0
adjacency_array.each_with_index do |adjacent_nodes, index|
  index_id = index + 1
  adjacent_nodes.each do |node|
    node_index = node - 1
    unless adjacency_array[node_index].include? index_id
      adjacency_array[node_index] << index_id
      added += 1
    end
  end
end
puts "done adding (added #{added} edges)"

puts 'exporting'
File.open("undirected.#{ARGV[0]}", 'w+') do |export|
  export.puts "#{nodes} #{adjacency_array.inject(0) { |result, adj| result + adj.size } / 2}"

  adjacency_array.each do |adjacent_nodes|
    export.puts adjacent_nodes.uniq.join(' ')
  end
end
puts 'done - bye'