# Removes any doubled edges from a graph
#
# First argument: a graph in the DIMACs .gr format
#
# Writes a file named 'simple.${input}' containing the given graph in the DIMACs .gr format where for each node pair only the edge with the minimal weight exists.

unless File.exists? ARGV[0]
  puts 'File not found! Exiting!'
  exit
end


n_declared = m_declared = m_counted = 0
doubled_edges = 0
edges = {}

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
      key = [source, target]
      weight = weight.to_i
      if edges.has_key? key
        edges[key] = [edges[key], weight].min
        doubled_edges += 1
      else
        edges[key] = weight
        m_counted += 1
      end
    else
      puts 'broken line'
  end
end

puts "found #{doubled_edges} doubled edges" unless doubled_edges == 0

puts 'exporting'
File.open("simple.#{ARGV[0]}", 'w+') do |export|
  export.puts "p sp #{n_declared} #{m_counted}"

  edges.each do |edge, weight|
    export.puts "a #{edge[0]} #{edge[1]} #{weight}"
  end
end

puts 'done - bye'