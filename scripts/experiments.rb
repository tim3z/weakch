# This is the script I used to run all sorts of different experiments with all possible combinations of different variants (which were activated through compile flags) on all avaiable graphs. 
# HORRIBLE. 
# And deprecated since i cleaned most of that mess away for publishing.
# Most stuff might still work, but some programms were renamed for clarity - so be very careful if you want to use any of this.

require 'yaml'

BUILD_DIR = File.absolute_path ARGV[0]
GRAPHS_DIR = File.absolute_path ARGV[1]

ND_METIS = 'lib/metis-5.1.0/programs/ndmetis'

puts 'build:', BUILD_DIR, 'graphs:', GRAPHS_DIR

def build *flags
  puts "building with flags #{flags.join ' '}"
	Dir.chdir(BUILD_DIR) do 
		`cmake .. -DFLAGS="#{flags.map { |flag| "-D#{flag}"}.join ' '}"`
		`make`
	end
end

def for_each_graph(&block)
  Dir.chdir(GRAPHS_DIR) do
    Dir.foreach(GRAPHS_DIR) do |graph|
      next if graph == '.' || graph == '..' || !File.directory?(graph)
      Dir.chdir(graph, &block)
    end
  end
end

def run(target, args)
  puts "> #{BUILD_DIR}/#{target} #{args}"
  `#{BUILD_DIR}/#{target} #{args}`
end
def run_script(target, args)
  puts "> ruby #{BUILD_DIR}/../scripts/#{target} #{args}"
  `ruby #{BUILD_DIR}/../scripts/#{target} #{args}`
end

class Experiments
  def initialize
    @flags = []
    @prefixes = []
  end

  def self.execute &block
    Experiments.new.instance_eval &block
  end

  def set(flag)
    @flags.push flag
    build *@flags
    yield
    @flags.pop
  end

  def for_each_flag(flags)
    flags.each do |key, flag|
      @prefixes.push key
      @flags.push flag unless flag == ''
      build *@flags
      yield key
      @prefixes.pop
      @flags.pop unless flag == ''
    end
  end

  def file(*args)
    if args.size == 2
      return args[0] + file(args[1])
    end
    "#{@prefixes.reverse.join('.')}.#{args[0]}"
  end
end

puts 'Preparing', '###################################################################################'
configs = {}
for_each_graph do |dir|
	configs[dir] = YAML::load File.read('config.yaml')
	Dir.mkdir 'contraction'
	Dir.mkdir 'customization'
	Dir.mkdir 'queries'
	Dir.mkdir 'changes'
	Dir.mkdir 'partial'
end
build

puts 'Preprocessing', '###################################################################################'
for_each_graph do |dir|
	graph = configs[dir]['time_metric']

	puts run_script 'gr_to_graph.rb', graph
	puts run_script 'undirect.rb', "#{graph}.graph"
	puts run ND_METIS, "undirected.#{graph}.graph -dbglvl=64 > metis_stats"
	puts run_script 'simplify.rb', graph
	puts run_script 'simplify.rb', configs[dir]['distance_metric']
	puts run_script 'apply_order.rb', "simple.#{graph} undirected.#{graph}.graph.iperm ordered.time.gr"
	puts run_script 'apply_order.rb', "simple.#{configs[dir]['distance_metric']} undirected.#{graph}.graph.iperm ordered.distance.gr"
end

puts 'Running Dijkstras', '###################################################################################'
Experiments.execute do
  set 'DIJKSTRA' do
    for_each_graph do |_|
      %w(time distance).each do |metric|
        puts run 'builder', "ordered.#{metric}.gr #{metric}.graph_plain"
      end
    end
    for_each_flag 'times' => '', 'stats' => 'STATS' do |stats|
      for_each_graph do |_|
        %w(time distance).each do |metric|
          run 'queries', "#{metric}.graph_plain _ #{file 'queries/', "#{metric}.dijkstra.csv"}"
        end
      end
    end
  end
end

puts 'Contraction', '###################################################################################'
File.open(GRAPHS_DIR + '/dags.csv', 'w+') do |dags|
File.open(GRAPHS_DIR + '/elimination_tree.csv', 'w+') do |tree|
	dags.puts 'file,nodes,arcs,time_in_ms,shortcuts_arcs'
	tree.puts 'file,nodes,edges,time_in_ms,shortcuts_edges'

	for_each_graph do |dir|
		dags.print configs[dir]['instance'], ','
    dags.puts run 'contraction', 'ordered.time.gr'

		tree.print configs[dir]['instance'], ','
    tree.puts run 'undirected_contraction', 'ordered.time.gr'
	end
end
end

build 'STATS'
for_each_graph do |_|
  puts run 'contraction', 'ordered.time.gr'
end

puts 'WeakCH', '###################################################################################'

File.open(GRAPHS_DIR + '/weak.macrocode.csv', 'w+') do |macrocode|
  macrocode.puts 'file,build_time_in_ms,instruction_time_in_ms'
  for_each_graph do |dir|
    macrocode.print configs[dir]['instance'], ','
    macrocode.puts run 'buildmacrocode', 'up.dag down.dag weak.macrocode'
  end
end

puts 'Customization', '###################################################################################'
File.open(GRAPHS_DIR + '/weak.customization.csv', 'w+') do |file|
  file.puts 'file,custom_time_in_ms,meta_time_in_ms'
  for_each_graph do |dir|
    %w(time distance).each do |metric|
      file.print "#{metric}.#{configs[dir]['instance']}", ','
      file.puts run('macrocustomization', "graph weak.macrocode ordered.#{metric}.gr #{metric}.weak.customized")
    end
  end
end
build 'CHANGE'
for_each_graph do |_|
  %w(time distance).each do |metric|
    puts run('macrocustomization', "graph weak.macrocode ordered.#{metric}.gr _ customization/#{metric}.weak.change_time.csv")
    puts run('delta', "#{metric}.weak.customized customization/#{metric}.weak.build_time.csv")
    `mv changes #{metric}.changes`
    Dir.mkdir 'changes'
  end
end

puts 'Queries', '###################################################################################'

Experiments.execute do
  for_each_flag 'meta' => '', 'nometa' => 'NOMETA' do |_|
    for_each_flag 'times' => '', 'stats' => 'STATS' do |stats|
      for_each_flag 'dist_label' => '', 'dist_array' => 'DIST_LABEL', 'dist_hash' => 'DIST_HASH' do |dist|
        if stats == 'times' || dist == 'dist_label'
          for_each_graph do |_|
            %w(time distance).each do |metric|
              run 'queries', "#{metric}.weak.customized eliminationtree #{file 'queries/', "#{metric}.weak.csv"}"
            end
          end
        end
      end
    end
  end
end






puts 'PlainCH', '###################################################################################'
Experiments.execute do
  set 'PLAINCH' do
    File.open(GRAPHS_DIR + '/plain.macrocode.csv', 'w+') do |macrocode|
      macrocode.puts 'file,build_time_in_ms,instruction_time_in_ms'
      for_each_graph do |dir|
        macrocode.print configs[dir]['instance'], ','
        macrocode.puts run 'buildmacrocode', 'up.dag down.dag plain.macrocode'
      end
    end

    puts 'Customization', '###################################################################################'
    File.open(GRAPHS_DIR + '/plain.customization.csv', 'w+') do |file|
      file.puts 'file,custom_time_in_ms,meta_time_in_ms'
      for_each_graph do |dir|
        %w(time distance).each do |metric|
          file.print "#{metric}.#{configs[dir]['instance']}", ','
          file.puts run('macrocustomization', "graph plain.macrocode ordered.#{metric}.gr #{metric}.plain.customized customization/#{metric}.plain.change_time.csv")
        end
      end
    end

    puts 'Queries', '###################################################################################'
    for_each_flag 'times' => '', 'stats' => 'STATS' do
      for_each_graph do |dir|
        %w(time distance).each do |metric|
          puts run 'queries', "#{metric}.plain.customized _ #{file 'queries/', "#{metric}.plain.csv"}"
        end
      end
    end
  end
end





puts 'Partial', '###################################################################################'
build
for_each_graph do |dir|
  File.open('partial/dags.csv', 'w+') do |dags|
  File.open('partial/elimination_tree.csv', 'w+') do |tree|
  File.open('partial/weak.macrocode.csv', 'w+') do |macrocode|
  File.open('partial/weak.customization.csv', 'w+') do |customization|
    dags.puts 'nodes,arcs,time_in_ms,shortcuts_arcs'
    tree.puts 'nodes,edges,time_in_ms,shortcuts_edges'
    macrocode.puts 'build_time_in_ms,instruction_time_in_ms'
    customization.puts 'custom_time_in_ms,meta_time_in_ms'

    5.times do |i|
      sep = configs[dir]['toplevel_sep'].to_i
      size = configs[dir]['uncontracted'] = sep * (2 ** i)
      File.open('config.yaml', 'w+') { |f| f.write(configs[dir].to_yaml) }

      dags.puts run 'contraction', 'ordered.time.gr'
      tree.puts run 'undirected_contraction', 'ordered.time.gr'
      macrocode.puts run 'buildmacrocode', "up.dag down.dag #{size}macrocode"

      customization.puts run 'macrocustomization', "graph #{size}macrocode ordered.time.gr #{size}time.customized"
    end
  end
  end
  end
  end
end

Experiments.execute do
  for_each_flag 'times' => '', 'stats' => 'STATS' do |stats|
    for_each_graph do |dir|
      5.times do |i|
        sep = configs[dir]['toplevel_sep'].to_i
        configs[dir]['uncontracted'] = size = sep * (2 ** i)
        File.open('config.yaml', 'w+') { |f| f.write(configs[dir].to_yaml) }

        run 'queries', "#{size}time.customized eliminationtree #{file 'queries/', "#{size}time.weak.csv"}"
      end

      configs[dir]['uncontracted'] = 0
      File.open('config.yaml', 'w+') { |f| f.write(configs[dir].to_yaml) }
    end
  end
end



puts 'Dijkstra Ranks', '###################################################################################'
Experiments.execute do
  set 'RANKS' do
    for_each_flag 'times' => '', 'stats' => 'STATS' do
      for_each_flag 'weak' => '', 'plain' => 'PLAINCH' do |algo|
        for_each_graph do
          %w(time distance).each do |metric|
            puts run 'queries', "#{metric}.#{algo}.customized eliminationtree #{file 'queries/', "#{metric}.ranks.csv"} dijkstra_ranks"
          end
        end
      end
    end
  end
end



puts 'Traditional Order', '###################################################################################'
build
Experiments.execute do
  set 'PLAINCH' do
    for_each_graph do |dir|
      %w(time distance).each do |metric|
        puts run 'chranks', "#{configs[dir]["#{metric}_metric"][0..-4]} > #{metric}.trad_contraction"
        puts run_script 'apply_order.rb', "traditional_ch_result.gr traditional_ch_ranks #{metric}.traditional_ordered.gr"
        puts run 'builder', "#{metric}.traditional_ordered.gr #{metric}.traditional_ordered"
      end
    end
    for_each_flag 'times' => '', 'stats' => 'STATS' do
      for_each_graph do
        %w(time distance).each do |metric|
          puts run 'queries', "#{metric}.traditional_ordered #{metric}.eliminationtree #{file 'queries/', "#{metric}.traditional.csv"}"
        end
      end
    end
  end
end
