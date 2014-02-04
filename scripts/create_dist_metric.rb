# Computes a distance metric from coordinates
#
# First argument: a graph in the DIMACs .gr format
# Second argument: coordinates in the DIMACs .co format
# Third argument: name of the output file
#
# Writes a file with the name of the third argument which contains the same graph with a new metric roughly equal to the actual distances between the nodes

unless File.exists?(ARGV[0]) && File.exists?(ARGV[1])
  puts 'File not found! Exiting!'
  exit
end

puts 'reading cords'
enumerator = File.open(ARGV[1]).each
nodes = enumerator.next.split(' ')[-1].to_i
coords = Array.new nodes + 1

enumerator.each do |line|
	line = line.split(' ')
	coords[line[1].to_i] = line[2..3].map { |coord| coord.to_i } if line[0] == 'v'
end

def distance x1, y1, x2, y2
	(((x1 - x2) ** 2 + (y1 - y2) ** 2) ** 0.5).round
end

puts 'reading graph'
File.open(ARGV[2], 'w+') do |file|
File.open(ARGV[0]).each do |line|
	if line[0] == 'a'
		line = line.split(' ')
		line[1] = line[1].to_i
		line[2] = line[2].to_i
		line[3] = distance coords[line[1]][0], coords[line[1]][1], coords[line[2]][0], coords[line[2]][1]
		file.puts line.join ' '
	else 
		file.puts line
	end
end
end