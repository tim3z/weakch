# Reorders the node ids of a graph
#
# First argument: a graph in the DIMACs .gr format
# Second argument: a node order for that graph in the format as ouput by metis (inverse permutation) - see the metis manual for the programm ndmetis
# Optional third argument: name of the output file
#
# Output: Writes a file with the name of the third argument (or 'ordered.gr') which contains the same graph which was given as input only that each nodes id equals its rank of the given node order

unless File.exists?(ARGV[0]) && File.exists?(ARGV[1])
  puts 'File not found! Exiting!'
  exit
end

puts 'reading ranks'
ranks = []
id = -1
File.open(ARGV[1]).each do |line|
  ranks[id += 1] = line.to_i
end

ARGV[2] ||= 'ordered.gr'
puts 'reading graph'
File.open(ARGV[2], 'w+') do |file|
  File.open(ARGV[0]).each do |line|
    type, source, target, weight, reason = line.split(' ')
    case type
      when 'c' then
        next
      when 'p' then
        puts "#{target} vertices, #{weight} edges"
        file.puts line
      when 'a'
        if reason && reason != '-1'
          file.puts "a #{ranks[source.to_i - 1]} #{ranks[target.to_i - 1]} #{weight} #{ranks[reason.to_i]}"
        else
          file.puts "a #{ranks[source.to_i - 1]} #{ranks[target.to_i - 1]} #{weight} -1"
        end
      else
        puts 'broken line'
    end
  end
end